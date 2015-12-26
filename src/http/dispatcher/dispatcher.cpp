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
#include <type_traits>

using namespace Web;
using namespace Cache;
static ResponseSerializer serializer;

class Dispatcher::DispatcherImpl {
    RouteUtility::RouteMap routes;
    std::vector<Http::Context> pending;

    public:
    DispatcherImpl() = default;

    inline void AddRoute(RouteUtility::Route r) noexcept { routes.push_back(r); }

    inline std::unique_ptr<MemoryBuffer> HandleBarrier(AsyncBuffer<Http::Response> *r) noexcept {
        auto http_response = r->future.get();
        return std::make_unique<MemoryBuffer>(serializer(http_response));
    }

    inline ScheduleItem HandleConnection(const IO::Socket *connection) noexcept {
        try {
            auto context_it = std::find_if(pending.begin(), pending.end(), [connection](Http::Context &engine) {
                return (*engine.GetSocket()) == (*connection);
            });
            if (context_it != pending.end())
                std::swap(*context_it, pending.back());
            else
                pending.emplace_back(connection);
            auto context = pending.back();
            if (context().Complete()) {
                pending.erase(pending.end() - 1);
                return ProcessRequest(context.GetRequest());
            }
        } catch (...) {
            return {};
        }
        return {};
    }

    private:
    inline ScheduleItem ProcessRequest(const Http::Request &r) const noexcept {
        if (Http::Util::IsResource(r))
            return TakeResource(r);
        if (Http::Util::IsPassable(r)) {
            if (auto user_handler = RouteUtility::GetUserHandler(r, routes))
                return PassRequest(r, user_handler);
            else
                return NotFound();
        } else {
            // TODO handle internally
            return {};
        }
    }

    inline ScheduleItem TakeResource(const Http::Request &request) const noexcept {
        auto full_path = Storage::GetSettings().root_path + request.url;
        try {
            if (ShouldCopyInMemory(full_path)) {
                if (auto resource = ResourceCache::Aquire(full_path)) {
                    Http::Response response{std::move(resource)};
                    return {serializer(response), response.GetKeepAlive()};
                } else
                    throw Http::StatusCode::NotFound;
            } else {
                auto unix_file = std::make_unique<UnixFile>(full_path, Cache::FileDescriptor::Aquire,
                                                            Cache::FileDescriptor::Release);
                ScheduleItem response;
                Http::Response http_response;
                http_response.SetFile(unix_file.get());
                http_response.Set(Http::Header::Fields::Content_Type, Http::Util::GetMimeType(full_path));
                http_response.Set(Http::Header::Fields::Content_Length, std::to_string(unix_file->size));
                response.PutBack(serializer.MakeHeader(http_response));
                response.PutBack(std::move(unix_file));
                response.PutBack(serializer.MakeEnding(http_response));
                response.SetKeepFileOpen(http_response.GetKeepAlive());
                return response;
            }
        } catch (...) {
            return NotFound();
        }
    }

    inline ScheduleItem PassRequest(const Http::Request &r, RouteUtility::HttpHandler h) const noexcept {
        Http::Resolution resolution = h(r);
        if (resolution.GetType() == Http::Resolution::Type::Sync)
            return {serializer(resolution.GetResponse()), resolution.GetResponse().GetKeepAlive()};
        else
            return {std::make_unique<AsyncBuffer<Http::Response>>(std::move(resolution.GetFuture()))};
    }

    bool ShouldCopyInMemory(const fs::path &resource_path) const {
        try {
            auto page_size = static_cast<std::size_t>(getpagesize());
            auto file_size = fs::file_size(resource_path);
            return file_size <= page_size;
        } catch (...) {
            throw Http::StatusCode::NotFound;
        }
    }

    inline ScheduleItem NotFound() const noexcept { return {serializer({Http::StatusCode::NotFound})}; }
};

void Dispatcher::AddRoute(RouteUtility::Route route) noexcept { impl->AddRoute(route); }

ScheduleItem Dispatcher::HandleConnection(const IO::Socket *connection) noexcept {
    return impl->HandleConnection(connection);
}

std::unique_ptr<MemoryBuffer> Dispatcher::HandleBarrier(AsyncBuffer<Http::Response> *item) noexcept {
    return impl->HandleBarrier(item);
}

Dispatcher::Dispatcher() : impl(nullptr) { impl = new DispatcherImpl(); }

Dispatcher::~Dispatcher() { delete impl; }

Dispatcher::Dispatcher(Dispatcher &&other) noexcept { *this = std::move(other); }
Dispatcher &Dispatcher::operator=(Dispatcher &&other) noexcept {
    if (this != &other) {
        impl = other.impl;
        other.impl = nullptr;
    }
    return *this;
}
