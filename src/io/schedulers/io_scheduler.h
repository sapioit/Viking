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

#include <io/schedulers/sys_epoll.h>
#include <io/schedulers/sched_item.h>
#include <io/socket/socket.h>
#include <io/buffers/mem_buffer.h>
#include <functional>

class Channel;

namespace IO {
class Scheduler {
    private:
    struct SocketNotFound {
        const SysEpoll::Event *event;
    };
    struct WriteError {};

    public:
    typedef ScheduleItem Resolution;
    typedef std::vector<char> DataType;
    typedef std::function<Resolution(const Socket *)> ReadCallback;
    typedef std::function<std::unique_ptr<MemoryBuffer>(ScheduleItem &)> BarrierCallback;

    private:
    std::vector<std::unique_ptr<Channel>> channels;
    SysEpoll poll;
    ReadCallback read_callback;
    BarrierCallback barrier_callback;

    public:
    Scheduler() = default;
    Scheduler(std::unique_ptr<Socket> sock, ReadCallback, BarrierCallback);
    ~Scheduler() = default;

    Scheduler(const Scheduler &) = delete;
    Scheduler &operator=(const Scheduler &) = delete;
    Scheduler(Scheduler &&) = default;
    Scheduler &operator=(Scheduler &&) = default;

    void Add(std::unique_ptr<Socket> socket, std::uint32_t flags);
    virtual void Run() noexcept;

    protected:
    inline void QueueItem(IO::Channel *, ScheduleItem &, bool) noexcept;
    void AddNewConnections(const Channel *) noexcept;
    void Remove(Channel *) noexcept;
    void ProcessWrite(Channel *socket) noexcept;
    void FillChannel(Channel *);
    inline bool CanWrite(const SysEpoll::Event &event) const noexcept;
    inline bool CanRead(const SysEpoll::Event &event) const noexcept;
    inline bool CanTerminate(const SysEpoll::Event &event) const noexcept;
};
}

#endif
