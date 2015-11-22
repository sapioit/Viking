#include <server/dispatcher.h>
#include <http/engine.h>
#include <http/util.h>
#include <http/response_serializer.h>
#include <cache/file_descriptor.h>
#include <io/filesystem.h>
#include <misc/storage.h>

RouteMap Web::Dispatcher::routes;
using namespace Web;

static ResponseSerializer serialize;

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
    auto response = handler(request);
    return {serialize(response), response.should_close()};
}

Dispatcher::SchedulerResponse Dispatcher::TakeResource(const Http::Request &request) noexcept {
    try {
        auto full_path = Storage::GetSettings().root_path + request.url;
        auto content_type = Http::Util::GetMimeType(request.url);
        if (ShouldCopyInMemory(full_path)) {
            auto resource = CacheManager::GetResource(request.url);
            Http::Response response{resource};
            response.SetContentType(content_type);
            return {serialize(response)};
        } else {
            auto unix_file =
                std::make_unique<UnixFile>(full_path, Cache::FileDescriptor::Aquire, Cache::FileDescriptor::Release);
            SchedulerResponse response;
            Http::Response http_response{unix_file.get()};
            http_response.SetContentType(content_type);
            auto header_str = http_response.header_str();
            response.AddData(std::vector<char>(header_str.begin(), header_str.end()));
            response.AddData(std::move(unix_file));
            auto header_end = http_response.end_str();
            response.AddData(std::vector<char>(header_end.begin(), header_end.end()));
            return response;
        }
    } catch (...) {
        return {serialize({Http::StatusCode::NotFound})};
    }
}

Dispatcher::SchedulerResponse Dispatcher::HandleConnection(const Connection &connection) {

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
            return {serialize({Http::StatusCode::NotFound})};
    }
    return {serialize({Http::StatusCode::NotFound})};
}
