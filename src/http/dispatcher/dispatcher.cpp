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

using namespace web;
using namespace cache;
static response_serializer serializer;

class dispatcher::dispatcher_impl {
    route_util::route_map routes;
    // std::vector<http::context> pending;

    public:
    dispatcher_impl() = default;

    inline void add_route(route_util::route r) noexcept { routes.push_back(r); }

    inline std::unique_ptr<io::memory_buffer> handle_barrier(async_buffer<http::response> *r) noexcept {
        auto http_response = r->future.get();
        return std::make_unique<io::memory_buffer>(serializer(http_response));
    }

    inline schedule_item handle_connection(io::channel *connection) {
        try {
            if (connection->state == nullptr)
                connection->state = new http::context(connection->socket.get());
            auto http_ctx = static_cast<http::context *>(connection->state);
            (*http_ctx)();
            if (http_ctx->complete()) {
                auto to_return = process_request(http_ctx->get_request());
                delete http_ctx;
                connection->state = nullptr;
                return to_return;
            }
        } catch (const io::tcp_socket::connection_closed_by_peer &) {
            throw;
        }
        return {};
    }

    private:
    inline schedule_item process_request(const http::request &r) const noexcept {
        if (http::util::is_disk_resource(r))
            return take_disk_resource(r);
        if (http::util::is_passable(r)) {
            if (auto user_handler = route_util::get_user_handler(r, routes))
                return pass_request(r, user_handler);
            else
                return not_found();
        } else {
            // TODO handle internally
            return {};
        }
    }

    bool should_keep_alive(const http::request &r) const noexcept {
        auto it = r.m_header.get_fields_c().find(http::header::fields::Connection);
        if (it != r.m_header.get_fields_c().end() && it->second == "Keep-Alive")
            return true;
        return false;
    }

    inline schedule_item take_folder(const http::request &request) const {
        auto &folder_cb = storage::config().folder_cb;
        auto resolution = folder_cb(request);
        return {serializer(resolution.get_response()), resolution.get_response().get_keep_alive()};
    }

    inline schedule_item take_file_from_memory(const http::request &request, fs::path full_path) const {
        if (auto resource = resource_cache::aquire(full_path)) {
            http::response response{std::move(resource)};
            response.set(http::header::fields::Connection, should_keep_alive(request) ? "Keep-Alive" : "Close");
            return {serializer(response), response.get_keep_alive()};
        } else {
            throw http::status_code::NotFound;
        }
    }

    inline schedule_item take_unix_file(const http::request &request, fs::path full_path) const {
        auto unix_file =
            std::make_unique<io::unix_file>(full_path, cache::file_descriptor::aquire, cache::file_descriptor::release);
        schedule_item response;
        http::response http_response;
        http_response.set_file(unix_file.get());
        http_response.set(http::header::fields::Content_Type, http::util::get_mimetype(full_path));
        http_response.set(http::header::fields::Content_Length, std::to_string(unix_file->size));
        http_response.set(http::header::fields::Connection, should_keep_alive(request) ? "Keep-Alive" : "Close");
        response.put_back(std::make_unique<io::memory_buffer>(serializer.make_header(http_response)));
        response.put_back(std::move(unix_file));
        response.put_back(std::make_unique<io::memory_buffer>(serializer.make_ending(http_response)));
        response.set_keep_file_open(http_response.get_keep_alive());
        return response;
    }

    inline schedule_item take_regular_file(const http::request &request, fs::path full_path) const {
        if (should_copy(full_path)) {
            return take_file_from_memory(request, full_path);
        } else {
            return take_unix_file(request, full_path);
        }
    }

    inline schedule_item take_disk_resource(const http::request &request) const noexcept {
        auto full_path = storage::config().root_path + request.url;
        try {
            if (fs::is_directory(full_path)) {
                return take_folder(request);
            } else if (fs::is_regular_file(full_path)) {
                return take_regular_file(request, full_path);
            }
        } catch (...) {
        }
        return not_found();
    }

    inline schedule_item pass_request(const http::request &r, route_util::http_handler h) const noexcept {
        http::resolution resolution = h(r);
        if (resolution.get_type() == http::resolution::type::sync)
            return {serializer(resolution.get_response()), resolution.get_response().get_keep_alive()};
        else
            return {std::make_unique<async_buffer<http::response>>(std::move(resolution.get_future()))};
    }

    bool should_copy(const fs::path &resource_path) const {
        try {
            auto page_size = static_cast<std::size_t>(getpagesize());
            auto file_size = fs::file_size(resource_path);
            return file_size <= page_size;
        } catch (...) {
            throw http::status_code::NotFound;
        }
    }

    inline schedule_item not_found() const noexcept { return schedule_item{serializer({http::status_code::NotFound})}; }
};

void dispatcher::add_route(route_util::route route) noexcept { impl->add_route(route); }

schedule_item dispatcher::handle_connection(io::channel *connection) {
    try {
        return impl->handle_connection(connection);
    } catch (...) {
        throw;
    }
}

std::unique_ptr<io::memory_buffer> dispatcher::handle_barrier(async_buffer<http::response> *item) noexcept {
    return impl->handle_barrier(item);
}

void dispatcher::will_remove(io::channel *s) noexcept {
    delete static_cast<http::context *>(s->state);
    s->state = nullptr;
}

dispatcher::dispatcher() : impl(nullptr) { impl = new dispatcher_impl(); }

dispatcher::~dispatcher() { delete impl; }

dispatcher::dispatcher(dispatcher &&other) noexcept { *this = std::move(other); }
dispatcher &dispatcher::operator=(dispatcher &&other) noexcept {
    if (this != &other) {
        impl = other.impl;
        other.impl = nullptr;
    }
    return *this;
}
