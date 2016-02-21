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
#ifndef CONTEXT_H
#define CONTEXT_H

#include <memory>
#include <io/socket/socket.h>
#include <io/schedulers/sched_item.h>

namespace io {
struct channel {
    std::unique_ptr<tcp_socket> socket;
    schedule_item queue;
    std::vector<std::uint32_t> journal;
    channel();
    channel(std::unique_ptr<tcp_socket> socket);
    channel(const channel &) = delete;
    channel &operator=(const channel &) = delete;
    channel(channel &&other) = default;
    channel &operator=(channel &other) = default;
    ~channel() = default;
    bool operator=(const channel &other) const;
};
}

#endif // CONTEXT_H
