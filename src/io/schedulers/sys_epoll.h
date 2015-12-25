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
#ifndef SYS_EPOLL_H
#define SYS_EPOLL_H

#include <sys/epoll.h>
#include <vector>
#include <io/schedulers/channel.h>

class SysEpoll {
    int efd_;
    std::vector<epoll_event> events_;

    epoll_event *FindEvent(const IO::Channel *socket);

    public:
    struct Event {
        public:
        IO::Channel *context;
        std::uint32_t description;
        Event() noexcept = default;
        Event(IO::Channel *, std::uint32_t description) noexcept;
        bool operator<(const Event &other) const { return context < other.context; }
        bool operator==(const Event &other) const { return context == other.context; }
    };

    struct PollError : public std::runtime_error {
        PollError(const std::string &err);
    };

    static constexpr std::uint32_t Read = EPOLLIN;
    static constexpr std::uint32_t Write = EPOLLOUT;
    static constexpr std::uint32_t Termination = EPOLLRDHUP;
    static constexpr std::uint32_t EdgeTriggered = EPOLLET;
    static constexpr std::uint32_t LevelTriggered = ~EPOLLET;
    static constexpr std::uint32_t Error = EPOLLERR;
    void Schedule(IO::Channel *, std::uint32_t);
    void Modify(const IO::Channel *, std::uint32_t);
    void Remove(const IO::Channel *);
    std::vector<Event> Wait(std::uint32_t = 1000) const;

    SysEpoll();
    virtual ~SysEpoll();
    SysEpoll(const SysEpoll &) = delete;
    SysEpoll &operator=(const SysEpoll &) = delete;
    SysEpoll(SysEpoll &&);
    SysEpoll &operator=(SysEpoll &&);
};

#endif
