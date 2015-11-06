#ifndef SYS_EPOLL_H
#define SYS_EPOLL_H

#include <sys/epoll.h>
#include <vector>
#include <stdexcept>

class SysEpoll {
        int efd_;
        std::vector<epoll_event> events_;

      public:
        struct Event {
              public:
                int file_descriptor;
                std::uint32_t description;
                Event() noexcept = default;
                Event(int fd, int description) noexcept
                    : file_descriptor(fd),
                      description(description) {}
        };

        struct Error : public std::runtime_error {
                Error(const std::string &err) : std::runtime_error(err) {}
        };
        enum class Description {
                Read = EPOLLIN,
                Write = EPOLLOUT,
                Error = EPOLLERR,
                Termination = EPOLLRDHUP
        };
        void Schedule(int, std::uint32_t);
        void Remove(int);
        std::vector<Event> Wait(std::uint32_t = 1000) const;
        virtual std::uint32_t GetBasicFlags() = 0;

        SysEpoll();
        virtual ~SysEpoll() = default;
};

#endif