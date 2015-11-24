#ifndef SYS_EPOLL_H
#define SYS_EPOLL_H

#include <sys/epoll.h>
#include <vector>
#include <stdexcept>
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

    struct Error : public std::runtime_error {
        Error(const std::string &err);
    };

    enum class Description { Read = EPOLLIN, Write = EPOLLOUT, Error = EPOLLERR, Termination = EPOLLRDHUP };
    void Schedule(IO::Channel *, std::uint32_t);
    void Modify(const IO::Channel *, std::uint32_t);
    void Remove(const IO::Channel *);
    std::vector<Event> Wait(std::uint32_t = 1000) const;

    SysEpoll();
    virtual ~SysEpoll();
};

#endif
