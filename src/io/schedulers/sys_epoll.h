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

class epoll {
    int efd_;
    std::vector<epoll_event> events_;

    epoll_event *FindEvent(const io::channel *socket);

    public:
    struct Event {
        public:
        io::channel *context;
        std::uint32_t description;
        Event() noexcept = default;
        Event(io::channel *, std::uint32_t description) noexcept;
        bool operator<(const Event &other) const { return context < other.context; }
        bool operator==(const Event &other) const { return context == other.context; }
        inline bool CanWrite() const noexcept { return (description & epoll::write); }
        inline bool CanRead() const noexcept { return (description & epoll::read); }
        inline bool CanTerminate() const noexcept {
            return (description & epoll::termination) || (description & epoll::Error);
        }
    };

    struct poll_error : public std::runtime_error {
        poll_error(const std::string &err);
    };

    static constexpr std::uint32_t read = EPOLLIN;
    static constexpr std::uint32_t write = EPOLLOUT;
    static constexpr std::uint32_t termination = EPOLLRDHUP;
    static constexpr std::uint32_t edge_triggered = EPOLLET;
    static constexpr std::uint32_t level_triggered = ~EPOLLET;
    static constexpr std::uint32_t Error = EPOLLERR;
    void schedule(io::channel *, std::uint32_t);
    void modify(const io::channel *, std::uint32_t);
    void remove(const io::channel *);
    std::vector<Event> Wait(std::uint32_t = 1000) const;

    epoll();
    virtual ~epoll();
    epoll(const epoll &) = delete;
    epoll &operator=(const epoll &) = delete;
    epoll(epoll &&);
    epoll &operator=(epoll &&);
};

#endif
