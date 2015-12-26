/*
Copyright (C) 2015 Voinea Constantin Vladimir

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/
#include <cache/resource_cache.h>
#include <cache/file_descriptor.h>
#include <http/dispatcher/dispatcher.h>
#include <http/engine.h>
#include <http/util.h>
#include <http/resolution.h>
#include <http/response_serializer.h>
#include <http/routeutility.h>
#include <http/engine.h>
#include <io/filesystem.h>
#include <io/socket/socket.h>
#include <io/buffers/asyncbuffer.h>
#include <inl/mime_types.h>
#include <misc/storage.h>
#include <misc/common.h>
#include <misc/debug.h>
#include <algorithm>

using namespace Web;
using namespace Cache;

static ResponseSerializer serializer;

void Dispatcher::AddRoute(RouteUtility::Route route) noexcept { routes.push_back(route); }

bool ShouldCopyInMemory(const std::string &resource_path) {
    try {
        auto page_size = static_cast<std::size_t>(getpagesize());
        auto file_size = fs::file_size(resource_path); // IO::FileSystem::GetFileSize(resource_path.c_str());
        return file_size <= page_size;
    } catch (...) {
        throw Http::StatusCode::NotFound;
    }
}

ScheduleItem Dispatcher::PassRequest(const Http::Request &request, Handler handler) noexcept {
    Http::Resolution resolution = handler(request);
    if (resolution.GetType() == Http::Resolution::Type::Sync)
        return {serializer(resolution.GetResponse()), resolution.GetResponse().GetKeepAlive()};
    else
        return {std::make_unique<AsyncBuffer<Http::Response>>(std::move(resolution.GetFuture()))};
}

Http::Engine *Dispatcher::GetPendingEngine(const IO::Socket *connection) {
    auto it = std::find_if(pending.begin(), pending.end(),
                           [connection](Http::Engine &engine) { return (*engine.GetSocket()) == (*connection); });
    return it == pending.end() ? nullptr : &*it;
}

ScheduleItem Dispatcher::ProcessEngine(const IO::Socket *connection, Http::Engine *engine, bool existed) {
    auto request = (*engine)();
    if (Http::Util::IsComplete(request)) {
        if (existed)
            pending.erase(std::remove_if(pending.begin(), pending.end(), [&connection](Http::Engine &engine) {
                              return (*engine.GetSocket()) == (*connection);
                          }), pending.end());
        if (Http::Util::IsPassable(request)) {
            auto full_path = Storage::GetSettings().root_path + request.url;
            if (fs::is_regular_file(full_path)) {
                if (filesystem::GetExtension(full_path) != "")
                    return TakeResource(request);
            }
            if (auto handler = RouteUtility::GetHandler(request, routes))
                return PassRequest(request, handler);
            else
                return {serializer({Http::StatusCode::NotFound})};
        }
        return {serializer({Http::StatusCode::NotFound})};
    } else {
        if (!existed)
            pending.push_back(*engine);
        return {};
    }
}

ScheduleItem Dispatcher::HandleConnection(const IO::Socket *connection) noexcept {
    /* TODO Should make this function pretty one day */
    if (auto engine = GetPendingEngine(connection)) {
        return ProcessEngine(connection, engine, true);
    } else {
        auto new_engine = Http::Engine(connection);
        return ProcessEngine(connection, &new_engine, false);
    }
}

ScheduleItem Dispatcher::TakeResource(const Http::Request &request) noexcept {
    try {
        auto full_path = Storage::GetSettings().root_path + request.url;
        if (ShouldCopyInMemory(full_path)) {
            if (auto resource = ResourceCache::Aquire(full_path)) {
                Http::Response response{resource};
                return {serializer(response), response.GetKeepAlive()};
            } else
                throw Http::StatusCode::NotFound;
        } else {
            auto unix_file =
                std::make_unique<UnixFile>(full_path, Cache::FileDescriptor::Aquire, Cache::FileDescriptor::Release);
            ScheduleItem response;
            Http::Response http_response;
            http_response.SetFile(unix_file.get());
            http_response.Set(Http::Header::Fields::Content_Type, mime_types[(filesystem::GetExtension(full_path))]);
            http_response.Set(Http::Header::Fields::Content_Length, std::to_string(unix_file->size));
            response.PutBack(serializer.MakeHeader(http_response));
            response.PutBack(std::move(unix_file));
            response.PutBack(serializer.MakeEnding(http_response));
            response.SetKeepFileOpen(http_response.GetKeepAlive());
            return response;
        }
    } catch (UnixFile::Error) {
        return {serializer({Http::StatusCode::NotFound})};
    } catch (Http::StatusCode) {
        return {serializer({Http::StatusCode::NotFound})};
    } catch (...) {
        return {serializer({Http::StatusCode::NotFound})};
    }
    return {};
}

std::unique_ptr<MemoryBuffer> Dispatcher::HandleBarrier(AsyncBuffer<Http::Response> *item) noexcept {
    auto http_response = item->future.get();
    return std::make_unique<MemoryBuffer>(serializer(http_response));
}
