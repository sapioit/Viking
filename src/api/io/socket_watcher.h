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
  SocketWatcher(const Sock &sock) : master_socket_(sock) {}
  ~SocketWatcher() = default;

  void Add(int file_descriptor) {
    FileWatcher::Add(file_descriptor);
    SysEpoll::Schedule(file_descriptor, SysEpoll::Description::Read |
                                            SysEpoll::Description::Termination);
  }

  void Remove(const Sock &socket) {
    FileWatcher::Remove([&socket](int file_descriptor) {
      return socket.get_fd() == file_descriptor;
    });
    SysEpoll::Remove(socket.get_fd());
  }

  template <class Callback> void Run(Callback callback) {
    const auto events = std::move(SysEpoll::Wait(watched_files_.size()));

    std::vector<shared_ptr<Sock>> active_sockets;

    for (const auto &event : events) {
      auto associated_socket = std::find_if(
          watched_files_.begin(), watched_files_.end(), [](const Sock &socket) {
            return event.file_descriptor == socket->get_fd();
          }) if (associated_socket != watched_files_.end()) {

        if (event.description & SysEpoll::Description::Termination ||
            event.description & SysEpoll::Description::Error)
          Remove(*associated_socket);
        if (event.description & SysEpoll::Description::Read)
          active_sockets.emplace_back(*associated_socket);
      }
    }

    callback(active_sockets);
  }
};
}

#endif