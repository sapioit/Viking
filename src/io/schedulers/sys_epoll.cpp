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
#include <io/schedulers/sys_epoll.h>
#include <io/socket/socket.h>
#include <misc/debug.h>
#include <misc/common.h>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include <errno.h>
#include <unistd.h>

epoll::epoll() {
    efd_ = epoll_create1(0);
    if (efd_ == -1)
        debug("Could not start polling");
}

epoll::~epoll() {
    /* We only close the epoll file descriptor, because epoll is aware of
     * sockets
     * that get closed */
    if (efd_ != -1)
        ::close(efd_);
}

void epoll::schedule(io::channel *channel, std::uint32_t flags) {
    channel->ev_ctx.data.ptr = channel;
    channel->ev_ctx.events = flags;
    if (unlikely(-1 == epoll_ctl(efd_, EPOLL_CTL_ADD, channel->socket->get_fd(), &channel->ev_ctx))) {
        if (errno == EEXIST) {
            modify(channel, flags);
        }
    } else
        channel->epoll_flags = flags;
}

void epoll::modify(io::channel *channel, std::uint32_t flags) {
    channel->ev_ctx.events = flags;
    if (-1 == epoll_ctl(efd_, EPOLL_CTL_MOD, channel->socket->get_fd(), &channel->ev_ctx)) {
        // WTF?
    } else
        channel->epoll_flags = flags;
}

void epoll::remove(io::channel *channel) {
    if (-1 == epoll_ctl(efd_, EPOLL_CTL_DEL, channel->socket->get_fd(), &channel->ev_ctx))
        throw poll_error("Could not remove the file with fd = " + std::to_string(channel->socket->get_fd()) +
                         " from the OS queue");
}

static std::vector<epoll::event> create_events(const std::vector<epoll_event> &events) noexcept {
    std::vector<epoll::event> epoll_events;
    for (const auto &event : events)
        epoll_events.emplace_back(static_cast<io::channel *>(event.data.ptr), event.events);
    return epoll_events;
}
std::vector<epoll::event> epoll::await(std::uint32_t chunk_size) const {
    std::vector<epoll_event> active_files;
    active_files.resize(chunk_size);

    auto events_number = epoll_wait(efd_, &active_files.front(), chunk_size, -1);

    if (-1 == events_number) {
        active_files.resize(0);
        if (errno != EINTR)
            throw poll_error("Could not poll for events. errno = " + std::to_string(errno));
    } else {
        active_files.resize(events_number);
    }

    return create_events(active_files);
}

epoll::event::event(io::channel *channel, std::uint32_t description) noexcept : context(channel),
                                                                                description(description),
                                                                                flags(0) {}

epoll::poll_error::poll_error(const std::string &err) : std::runtime_error(err) {}

epoll &epoll::operator=(epoll &&other) {
    if (this != &other) {
        this->efd_ = other.efd_;
        other.efd_ = -1;
    }
    return *this;
}

epoll::epoll(epoll &&other) { *this = std::move(other); }
