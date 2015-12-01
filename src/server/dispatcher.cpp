#include <server/dispatcher.h>
#include <http/engine.h>
#include <cache/file_descriptor.h>
#include <io/filesystem.h>

RouteMap Web::Dispatcher::routes;

bool ShouldCopyInMemory(const std::string &resource_path) {
    try {
        auto page_size = static_cast<std::size_t>(getpagesize());
        auto file_size = IO::FileSystem::GetFileSize(resource_path.c_str());
        return file_size <= page_size;
    } catch (...) {
        throw Http::StatusCode::NotFound;
    }
}

Web::Dispatcher::SchedulerResponse Web::Dispatcher::Dispatch(const Connection &connection) {
    auto parser = Http::Engine(connection);
    auto request = parser();
    if (request.IsPassable()) {
        if (!request.IsResource()) {
            auto handler = RouteUtility::GetHandler(request, routes);
            if (handler) {
                auto response = handler(request);
                return {ResponseSerializer::Serialize(response), response.should_close()};
            } else {
                /* No user-defined handler, return not found */
                return {ResponseSerializer::Serialize({request, Http::StatusCode::NotFound})};
            }
        } else {
            /* It's a resource */
            try {
                auto full_path = Storage::settings().root_path + request.url;
                if (ShouldCopyInMemory(full_path)) {
                    auto resource = CacheManager::GetResource(request.url);
                    // TODO decide if you should really
                    // close the connection
                    return {ResponseSerializer::Serialize(request, resource)};
                } else {
                    try {
                        auto unix_file = std::make_unique<UnixFile>(full_path, Cache::FileDescriptor::Aquire,
                                                                    Cache::FileDescriptor::Release);
                        SchedulerResponse response;

                        Http::Response http_response{request, unix_file.get()};
                        http_response.SetContentType(Http::Engine::GetMimeTypeByExtension(request.url));

                        auto header_str = http_response.header_str();
                        response.AddData(std::vector<char>(header_str.begin(), header_str.end()));
                        response.AddData(std::move(unix_file));
                        auto header_end = http_response.end_str();
                        response.AddData(std::vector<char>(header_end.begin(), header_end.end()));

                        return response;
                    } catch (const UnixFile::Error &) {
                        return {ResponseSerializer::Serialize({request, Http::StatusCode::NotFound})};
                    }
                }
            } catch (Http::StatusCode code) {
                return {ResponseSerializer::Serialize({request, code})};
            }
        }
    } else {
        /* The request is not passable to the user and
    * it is not a resource either
        */
        return {ResponseSerializer::Serialize({request, Http::StatusCode::NotFound})};
    }
    return {};
}
