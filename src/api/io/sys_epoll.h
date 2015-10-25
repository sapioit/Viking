#ifndef SYS_EPOLL_H
#define SYS_EPOLL_H

#include <sys/epoll.h>
#include <vector>

class SysEpoll {
  int efd_;
  std::vector<epoll_event> events_;

public:
  struct Event {
  public:
    int file_descriptor;
    std::uint32_t description;
    Event() noexcept = default;
    Event(int fd, int description) noexcept : fd(fd),
                                              description(description) {}
  };

  struct Error : std::exception {
    Error(const std::string &err) : std::exception(err.c_str()) {}
  };
  enum class Description {
    Read = EPOLLIN,
    Write = EPOLLOUT,
    Error = EPOLLERR,
    Termination = EPOLLRDHUP
  };
  Schedule(int, std::uint32_t);
  Remove(int);
  auto Wait(std::uint32_t = 1000) const;

  SysEpoll();
  virtual ~SysEpoll() = default;
};

#endif