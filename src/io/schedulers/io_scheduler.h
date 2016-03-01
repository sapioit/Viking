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
#ifndef SOCKET_WATCHER_H
#define SOCKET_WATCHER_H

#include <io/schedulers/sched_item.h>
#include <io/schedulers/channel.h>
#include <io/socket/socket.h>
#include <io/buffers/mem_buffer.h>
#include <functional>

namespace io {
class scheduler {
    public:
    typedef schedule_item Resolution;
    typedef std::vector<char> DataType;
    typedef std::function<Resolution(channel *)> read_cb;
    typedef std::function<std::unique_ptr<memory_buffer>(schedule_item &)> barrier_cb;
    typedef std::function<void(channel *)> before_removing_cb;

    private:
    class scheduler_impl;
    scheduler_impl *impl;

    public:
    scheduler();
    scheduler(std::unique_ptr<tcp_socket> sock, read_cb, barrier_cb, before_removing_cb);
    ~scheduler();

    scheduler(const scheduler &) = delete;
    scheduler &operator=(const scheduler &) = delete;
    scheduler(scheduler &&);
    scheduler &operator=(scheduler &&);

    void add(std::unique_ptr<tcp_socket> socket, std::uint32_t flags);
    void run() noexcept;
};
}

#endif
