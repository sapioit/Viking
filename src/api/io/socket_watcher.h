#ifndef SOCKET_WATCHER_H
#define SOCKET_WATCHER_H

#include <io/file_watcher.h>
#include <io/socket.h>
#include <io/sys_epoll.h>
#include <algorithm>

namespace IO {
template <class Sock>
class SocketWatcher : public FileWatcher<Sock>, public SysEpoll {
  std::shared_ptr<Sock> master_socket_;

public:
  SocketWatcher(const std::shared_ptr<Sock> sock) : master_socket_(sock) {
    try {
      FileWatcher<Sock>::Add(sock);
      Schedule((*sock).get_fd(), static_cast<std::uint32_t>(SysEpoll::Description::Read) |
                              static_cast<std::uint32_t>(SysEpoll::Description::Termination));
    }
    catch(SysEpoll::Error) {
      throw;
    }
  }
  ~SocketWatcher() = default;

  void Add(int file_descriptor) {
    FileWatcher<Sock>::Add(file_descriptor);
    SysEpoll::Schedule(
        file_descriptor,
        static_cast<std::uint32_t>(SysEpoll::Description::Read) |
            static_cast<std::uint32_t>(SysEpoll::Description::Termination));
  }

  void Remove(const Sock &socket) {
    FileWatcher<Sock>::Remove([&socket](std::shared_ptr<Sock> candidate) {
      return socket.get_fd() == (*candidate).get_fd();
    });
    SysEpoll::Remove(socket.get_fd());
  }

  void Run(std::function<void(std::vector<std::shared_ptr<Sock>>)> callback) {
    const auto events =
        std::move(SysEpoll::Wait(FileWatcher<Sock>::watched_files_.size()));

    std::vector<std::shared_ptr<Sock>> active_sockets;

    for (const auto &event : events) {
      auto associated_socket =
          std::find_if(FileWatcher<Sock>::watched_files_.begin(),
                       FileWatcher<Sock>::watched_files_.end(),
                       [this, &event](const std::shared_ptr<Sock> socket) {
                         return event.file_descriptor == (*socket).get_fd();
                       });
      if (associated_socket != FileWatcher<Sock>::watched_files_.end()) {

        if (event.description & static_cast<std::uint32_t>(
                                    SysEpoll::Description::Termination) ||
            event.description &
                static_cast<std::uint32_t>(SysEpoll::Description::Error))
          Remove((*(*associated_socket)));
        if (event.description &
            static_cast<std::uint32_t>(SysEpoll::Description::Read))
          active_sockets.emplace_back((*associated_socket));
      }
    }

    callback(active_sockets);
  }
};
}

#endif