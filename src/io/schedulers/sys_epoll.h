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

#include <io/schedulers/channel.h>
#include <sys/epoll.h>
#include <vector>

class epoll {
    int efd_;
    std::vector<epoll_event> events_;

    epoll_event *find_event(const io::channel *socket);

    public:
    struct event {
        public:
        io::channel *context;
        std::uint32_t description;
        event() noexcept = default;
        event(io::channel *, std::uint32_t description) noexcept;
        bool operator<(const event &other) const { return context < other.context; }
        bool operator==(const event &other) const { return context == other.context; }
        inline bool can_write() const noexcept { return (description & epoll::write); }
        inline bool can_read() const noexcept { return (description & epoll::read); }
        inline bool can_terminate() const noexcept { return description & epoll::termination; }
        inline bool has_error() const noexcept { return description & epoll::error; }
    };

    struct poll_error : public std::runtime_error {
        poll_error(const std::string &err);
    };

    static constexpr std::uint32_t read = EPOLLIN;
    static constexpr std::uint32_t write = EPOLLOUT;
    static constexpr std::uint32_t termination = EPOLLRDHUP;
    static constexpr std::uint32_t edge_triggered = EPOLLET;
    static constexpr std::uint32_t error = EPOLLERR;
    void schedule(io::channel *);
    void update(const io::channel *);
    void remove(const io::channel *);
    std::vector<event> await(std::uint32_t = 1000) const;

    epoll();
    virtual ~epoll();
    epoll(const epoll &) = delete;
    epoll &operator=(const epoll &) = delete;
    epoll(epoll &&);
    epoll &operator=(epoll &&);
};

#endif
