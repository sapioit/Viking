#ifndef SOCKET_WATCHER_H
#define SOCKET_WATCHER_H

#include <io/watchers/file_watcher.h>
#include <io/socket/socket.h>
#include <io/watchers/sys_epoll.h>
#include <misc/debug.h>
#include <algorithm>

namespace IO {
template <class Sock>
class SocketWatcher : public FileWatcher<Sock>, public SysEpoll {
protected:
  std::shared_ptr<Sock> master_socket_;

public:
  virtual std::uint32_t GetBasicFlags() {
    return (static_cast<std::uint32_t>(SysEpoll::Description::Read) |
            static_cast<std::uint32_t>(SysEpoll::Description::Termination));
  }

  SocketWatcher(const std::shared_ptr<Sock> sock) : master_socket_(sock) {
    try {
      SocketWatcher<Sock>::Add(sock);
    } catch (const SysEpoll::Error &) {
      throw;
    }
  }
  ~SocketWatcher() = default;

  void Add(std::shared_ptr<IO::Socket> socket) {
    try {
      FileWatcher<Sock>::Add(socket);
      SysEpoll::Schedule((*socket).get_fd(), GetBasicFlags());
    } catch (const SysEpoll::Error &) {
      throw;
    }
  }

  void Remove(const Sock &socket) {
    FileWatcher<Sock>::Remove(socket);
    SysEpoll::Remove(socket.get_fd());
  }

  void Run(std::function<void(std::vector<std::shared_ptr<Sock>>)> callback) {
    const auto events =
        std::move(SysEpoll::Wait(FileWatcher<Sock>::watched_files_.size()));

    std::vector<std::weak_ptr<Sock>> active_sockets;

    for (const auto &event : events) {
      auto sock_it =
          std::find_if(FileWatcher<Sock>::watched_files_.begin(),
                       FileWatcher<Sock>::watched_files_.end(),
                       [this, &event](std::weak_ptr<Sock> socket) {
                         if (auto sock_ptr = socket.lock()) {
                           if (event.file_descriptor == (*sock_ptr).get_fd()) {
                             debug("Matched the event with fd = " +
                                   std::to_string(event.file_descriptor) +
                                   " with an active socket");
                             return true;
                           }
                         }
                         return false;
                       });
      if (sock_it != FileWatcher<Sock>::watched_files_.end()) {

        std::weak_ptr<Sock> associated_socket = (*sock_it);

        if ((*associated_socket)) == (*master_socket_)) {
            AddNewConnections();
            continue;
          }

        if (event.description & static_cast<std::uint32_t>(
                                    SysEpoll::Description::Termination) ||
            event.description &
                static_cast<std::uint32_t>(SysEpoll::Description::Error)) {
          debug("Event with fd = " + std::to_string(event.file_descriptor) +
                " must be closed");
          Remove((*associated_socket));
        }
        if (event.description &
            static_cast<std::uint32_t>(SysEpoll::Description::Read)) {
          debug("Socket with fd = " +
                std::to_string((*associated_socket).get_fd()) +
                " can be read from");
          active_sockets.emplace_back(associated_socket);
        }
      }

      debug("We have " + std::to_string(active_sockets.size()) +
            " active sockets");

      callback(active_sockets);
    }
  }

  void AddNewConnections() noexcept {
    while (true) {
      auto new_connection = (*master_socket_).Accept();
      if ((*new_connection).get_fd() == -1)
        break;
      (*new_connection).MakeNonBlocking();
      SocketWatcher<Sock>::Add(new_connection);
    }
  }
};
}

#endif