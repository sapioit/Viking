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
#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <io/schedulers/sched_item.h>
#include <io/socket/socket.h>
#include <io/schedulers/channel.h>
#include <http/resolution.h>
#include <http/routeutility.h>

#include <memory>
#include <functional>

namespace web {
class dispatcher {
    class dispatcher_impl;
    dispatcher_impl *impl;

    public:
    dispatcher();
    ~dispatcher();
    dispatcher(dispatcher &) = delete;
    dispatcher &operator=(dispatcher &) = delete;
    dispatcher(dispatcher &&) noexcept;
    dispatcher &operator=(dispatcher &&) noexcept;

    void add_route(route_util::route) noexcept;
    schedule_item handle_connection(io::channel *);
    std::unique_ptr<io::memory_buffer> handle_barrier(async_buffer<http::response> *) noexcept;
    void will_remove(io::channel *) noexcept;
};
}

#endif // DISPATCHER_H
