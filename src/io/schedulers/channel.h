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
#include <functional>

namespace IO {
struct Channel {
    std::unique_ptr<Socket> socket;
    ScheduleItem queue;
    std::vector<std::uint32_t> journal;
    void* data;
    std::uint8_t flags;
    static constexpr std::uint8_t Tainted = 1 << 1;
    std::function<void(Channel*)> on_destroy;
    Channel();
    Channel(std::unique_ptr<Socket> socket);
    Channel(const Channel &) = delete;
    Channel &operator=(const Channel &) = delete;
    Channel(Channel &&other) = default;
    Channel &operator=(Channel &other) = default;
    ~Channel();
    bool operator=(const Channel &other) const;
};
}

#endif // CONTEXT_H
