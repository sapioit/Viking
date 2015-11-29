#include <http/dispatcher/dispatcher.h>
#include <http/engine.h>
#include <http/util.h>
#include <http/resolution.h>
#include <http/response_serializer.h>
#include <cache/file_descriptor.h>
#include <io/filesystem.h>
#include <io/buffers/asyncbuffer.h>
#include <misc/storage.h>
#include <misc/common.h>

RouteMap Web::Dispatcher::routes;
using namespace Web;

static ResponseSerializer serializer;

bool ShouldCopyInMemory(const std::string &resource_path) {
    try {
        auto page_size = static_cast<std::size_t>(getpagesize());
        auto file_size = IO::FileSystem::GetFileSize(resource_path.c_str());
        return file_size <= page_size;
    } catch (...) {
        throw Http::StatusCode::NotFound;
    }
}

Dispatcher::SchedulerResponse Dispatcher::PassRequest(const Http::Request &request, Handler handler) noexcept {
    Http::Resolution resolution = handler(request);
    if (likely(resolution.GetType() == Http::Resolution::Type::Sync))
        return {serializer(resolution.GetResponse()), resolution.GetResponse().GetKeepAlive()};
    else
        return {std::move(std::make_unique<AsyncBuffer<Http::Response>>(std::move(resolution.GetFuture())))};
}

Dispatcher::SchedulerResponse Dispatcher::TakeResource(const Http::Request &request) noexcept {
    try {
        auto full_path = Storage::GetSettings().root_path + request.url;
        auto content_type = Http::Util::GetMimeType(request.url);
        if (ShouldCopyInMemory(full_path)) {
            auto resource = CacheManager::GetResource(request.url);
            Http::Response response{resource};
            response.SetContentType(content_type);
            response.SetCachePolicy({Storage::GetSettings().default_max_age});
            return {serializer(response), response.GetKeepAlive()};
        } else {
            auto unix_file =
                std::make_unique<UnixFile>(full_path, Cache::FileDescriptor::Aquire, Cache::FileDescriptor::Release);
            SchedulerResponse response;
            Http::Response http_response{unix_file.get()};
            http_response.SetContentType(content_type);
            http_response.SetCachePolicy({Storage::GetSettings().default_max_age});
            response.PutBack(serializer.MakeHeader(http_response));
            response.PutBack(std::move(unix_file));
            response.PutBack(serializer.MakeEnding(http_response));
            response.SetKeepFileOpen(http_response.GetKeepAlive());
            return response;
        }
    } catch (...) {
        return {serializer({Http::StatusCode::NotFound})};
    }
}

Dispatcher::SchedulerResponse Dispatcher::HandleConnection(const Connection *connection) noexcept {

    auto parser = Http::Engine(connection);
    auto request = parser();
    if (Http::Util::IsPassable(request)) {
        if (Http::Util::ExtensionAllowed(request.url))
            /* It's a resource on the filesystem */
            return TakeResource(request);
        if (auto handler = RouteUtility::GetHandler(request, routes))
            /* Got handler */
            return PassRequest(request, handler);
        else
            /* No handler */
            return {serializer({Http::StatusCode::NotFound})};
    }
    return {serializer({Http::StatusCode::NotFound})};
}

bool Dispatcher::HandleBarrier(ScheduleItem &item, std::unique_ptr<MemoryBuffer> &new_item) noexcept {
    auto raw_buffer = item.Front();
    if (likely(typeid(*raw_buffer) == typeid(AsyncBuffer<Http::Response>))) {
        auto async_buffer = dynamic_cast<AsyncBuffer<Http::Response> *>(raw_buffer);
        if (async_buffer->IsReady()) {
            auto http_response = async_buffer->future.get();
            new_item = std::make_unique<MemoryBuffer>(serializer(http_response));
            return true;
        }
    }
    return false;
}
