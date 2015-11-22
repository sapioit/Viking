#ifndef SYS_EPOLL_H
#define SYS_EPOLL_H

#include <sys/epoll.h>
#include <vector>
#include <stdexcept>
#include <io/socket/socket.h>

class SysEpoll {
    int efd_;
    std::vector<epoll_event> events_;

    epoll_event *FindEvent(const IO::Socket *socket);

    public:
    struct Event {
        public:
        IO::Socket *socket;
        std::uint32_t description;
        Event() noexcept = default;
        Event(IO::Socket *sock, int description) noexcept;
        bool operator<(const Event &other) const { return socket->GetFD() < other.socket->GetFD(); }
        bool operator==(const Event &other) const { return socket->GetFD() == other.socket->GetFD(); }
    };

    struct Error : public std::runtime_error {
        Error(const std::string &err);
    };

    enum class Description { Read = EPOLLIN, Write = EPOLLOUT, Error = EPOLLERR, Termination = EPOLLRDHUP };
    void Schedule(IO::Socket *, std::uint32_t);
    void Modify(const IO::Socket *, std::uint32_t);
    void Remove(const IO::Socket *);
    std::vector<Event> Wait(std::uint32_t = 1000) const;

    SysEpoll();
    virtual ~SysEpoll();
};

#endif
