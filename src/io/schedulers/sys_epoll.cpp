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
#include <algorithm>
#include <cstring>
#include <errno.h>
#include <io/schedulers/sys_epoll.h>
#include <io/socket/socket.h>
#include <misc/debug.h>
#include <stdexcept>
#include <unistd.h>

epoll::epoll() {
    efd_ = epoll_create1(0);
    if (efd_ == -1)
        debug("Could not start polling");
}

epoll::~epoll() {
    /* We only close the epoll file descriptor, because epoll is aware of
     * sockets that get closed */
    if (efd_ != -1)
        ::close(efd_);
}

void epoll::schedule(io::channel *context) {
    auto event = new epoll_event;
    std::memset(event, 0, sizeof(*event));
    context->cookie_1 = event;

    event->data.ptr = context;
    event->events = context->flags;
    if (-1 == epoll_ctl(efd_, EPOLL_CTL_ADD, context->socket->get_fd(), event)) {
        if (errno == EEXIST) {
            update(context);
        }
    }
}

void epoll::update(const io::channel *context) {
    epoll_event *event = static_cast<epoll_event *>(context->cookie_1);
    if (event) {
        event->events = context->flags;
        if (-1 == epoll_ctl(efd_, EPOLL_CTL_MOD, context->socket->get_fd(), event)) {
            debug("Could not modify the epoll parameters for file with fd = " << context->socket->get_fd());
        }
    }
}

void epoll::remove(io::channel *context) {
    auto event = static_cast<epoll_event *>(context->cookie_1);
    if (-1 == epoll_ctl(efd_, EPOLL_CTL_DEL, context->socket->get_fd(), event))
        debug("Could not remove the file with fd = " << context->socket->get_fd() << " from the OS queue");
    delete event;
    context->cookie_1 = nullptr;
}

static std::vector<epoll::event> create_events(const std::vector<epoll_event> &events) noexcept {
    std::vector<epoll::event> epoll_events;
    for (const auto &event : events) {
        epoll_events.emplace_back(static_cast<io::channel *>(event.data.ptr), event.events);
    }
    return epoll_events;
}

std::vector<epoll::event> epoll::await(std::uint32_t chunk_size) const {
    std::vector<epoll_event> active_files;
    active_files.resize(chunk_size);

    auto events_number = epoll_wait(efd_, &active_files.front(), chunk_size, 1000);

    if (-1 == events_number) {
        active_files.resize(0);
        if (errno != EINTR)
            throw poll_error("Could not poll for events. errno = " + std::to_string(errno));
    } else {
        active_files.resize(events_number);
    }

    return create_events(active_files);
}

epoll::event::event(io::channel *context, std::uint32_t description) noexcept : context(context),
                                                                                description(description) {}

epoll::poll_error::poll_error(const std::string &err) : std::runtime_error(err) {}

epoll &epoll::operator=(epoll &&other) {
    if (this != &other) {
        this->efd_ = other.efd_;
        other.efd_ = -1;
    }
    return *this;
}

epoll::epoll(epoll &&other) { *this = std::move(other); }
