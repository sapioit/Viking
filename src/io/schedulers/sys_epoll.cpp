#include <io/schedulers/sys_epoll.h>
#include <misc/debug.h>
#include <cstring>
#include <algorithm>
#include <errno.h>
#include <unistd.h>
#include <io/socket/socket.h>
SysEpoll::SysEpoll() {
    efd_ = epoll_create1(0);
    if (efd_ == -1)
        throw Error("Could not start polling");
    debug("SysEpoll instance with fd = " + std::to_string(efd_));
}

SysEpoll::~SysEpoll() {
    /* We only close the epoll file descriptor, because epoll is aware of
     * sockets
     * that get closed */
    ::close(efd_);
}

void SysEpoll::Schedule(IO::Channel *context, std::uint32_t flags) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(struct epoll_event));
    // ev.data.fd = file_descriptor;
    ev.data.ptr = context;
    ev.events = flags | EPOLLET;
    if (-1 == epoll_ctl(efd_, EPOLL_CTL_ADD, context->socket->GetFD(), &ev)) {
        if (errno != EEXIST) {
        } else {
            Modify(context, flags);
        }
    } else
        events_.push_back(ev);
}

void SysEpoll::Modify(const IO::Channel *context, std::uint32_t flags) {
    auto *event = FindEvent(context);
    if (event) {
        event->events |= flags;
        if (-1 == epoll_ctl(efd_, EPOLL_CTL_MOD, context->socket->GetFD(), event)) {
            debug("Could not modify the event with fd = " + std::to_string(file_descriptor) + " errno = " +
                  std::to_string(errno));
        }
    }
}

void SysEpoll::Remove(const IO::Channel *context) {
    // FIXME
    auto event_it =
        std::find_if(events_.begin(), events_.end(), [context](epoll_event &ev) { return (context == ev.data.ptr); });

    if (event_it != events_.end()) {
        auto *event = std::addressof(*event_it);
        if (-1 == epoll_ctl(efd_, EPOLL_CTL_DEL, context->socket->GetFD(), event))
            throw Error("Could not remove the file with fd = " + std::to_string(context->socket->GetFD()) +
                        " from the OS queue");

        events_.erase(std::remove_if(events_.begin(), events_.end(), [&event_it](auto &ev) {
                          return ev.data.fd == event_it->data.fd;
                      }), events_.end());
    } else {
        debug("SysEpoll could not find event with fd = " + std::to_string(file_descriptor));
    }
}

static std::vector<SysEpoll::Event> CreateEvents(const std::vector<epoll_event> &events) noexcept {
    std::vector<SysEpoll::Event> epoll_events;
    for (const auto &event : events) {
        epoll_events.emplace_back(static_cast<IO::Channel *>(event.data.ptr), event.events);
    }
    return epoll_events;
}
std::vector<SysEpoll::Event> SysEpoll::Wait(std::uint32_t chunk_size) const {
    std::vector<epoll_event> active_files;
    active_files.resize(chunk_size);

    auto events_number = epoll_wait(efd_, &active_files.front(), chunk_size, -1);

    if (-1 == events_number) {
        active_files.resize(0);
        if (errno != EINTR)
            throw Error("Could not poll for events. errno = " + std::to_string(errno));
    } else {
        debug("Waited for " + std::to_string(events_number) + " events");
        active_files.resize(events_number);
    }

    return CreateEvents(active_files);
}

epoll_event *SysEpoll::FindEvent(const IO::Channel *channel) {
    auto event_it = std::find_if(events_.begin(), events_.end(), [channel](const epoll_event &ev) {
        return (channel == ev.data.ptr);
    });
    return (event_it == events_.end() ? nullptr : std::addressof(*event_it));
}
SysEpoll::Event::Event(IO::Channel *context, std::uint32_t description) noexcept : context(context),
                                                                                   description(description) {}

SysEpoll::Error::Error(const std::string &err) : std::runtime_error(err) {}
