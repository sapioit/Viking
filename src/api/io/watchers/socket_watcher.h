#ifndef SOCKET_WATCHER_H
#define SOCKET_WATCHER_H

#include <io/watchers/file_watcher.h>
#include <io/socket/socket.h>
#include <io/watchers/sys_epoll.h>
#include <util/set.h>

#include <misc/debug.h>
#include <stdexcept>
#include <algorithm>
#include <utility>

namespace IO {
template <class Sock>
class SocketWatcher : public FileWatcher<Sock>, public SysEpoll {
      protected:
      public:
        virtual std::uint32_t GetBasicFlags() {
                return (
                    static_cast<std::uint32_t>(SysEpoll::Description::Read) |
                    static_cast<std::uint32_t>(
                        SysEpoll::Description::Termination));
        }

        SocketWatcher(std::unique_ptr<Sock> sock) {
                try {
                        SocketWatcher<Sock>::Add(std::move((*sock)));
                } catch (const SysEpoll::Error &) {
                        throw;
                }
        }
        SocketWatcher(Sock sock) {
                try {
                        SocketWatcher<Sock>::Add(std::move(sock));
                } catch (const SysEpoll::Error &) {
                        throw;
                }
        }
        ~SocketWatcher() = default;

        void Add(Sock socket) {
                try {
                        auto fd = socket.GetFD();
                        FileWatcher<Sock>::Add(std::move(socket));
                        SysEpoll::Schedule(fd, GetBasicFlags());
                } catch (const SysEpoll::Error &) {
                        throw;
                }
        }

        void Remove(const Sock &socket) {
                SysEpoll::Remove(socket.GetFD());
                FileWatcher<Sock>::Remove(socket);
        }

        virtual void Run(std::function<bool(const Sock &)> callback) {
                if (FileWatcher<Sock>::watched_files_.size() == 0)
                        return;
                const auto events = std::move(
                    SysEpoll::Wait(FileWatcher<Sock>::watched_files_.size()));

                std::vector<std::pair<const Sock *, const Event *>>
                    active_sockets;

                Utility::SetIntersection(
                    FileWatcher<Sock>::watched_files_.begin(),
                    FileWatcher<Sock>::watched_files_.end(), events.begin(),
                    events.end(), std::back_inserter(active_sockets),
                    SysEpoll::EventComparer<Sock>(),
                    Utility::Merge<const IO::Socket, const Event>);

                if (active_sockets.size() == 0 &&
                    (*FileWatcher<Sock>::watched_files_.begin()).IsAcceptor()) {
                        debug("The master socket is active");
                        AddNewConnections(
                            *FileWatcher<Sock>::watched_files_.begin());
                }

                for (const auto &event : active_sockets) {
                        const auto &associated_socket = *event.first;
                        const auto &associated_event = *event.second;

                        if (associated_socket.IsAcceptor()) {
                                debug("The master socket is active");
                                AddNewConnections(associated_socket);
                                continue;
                        }

                        if (associated_event.description &
                                static_cast<std::uint32_t>(
                                    SysEpoll::Description::Termination) ||
                            associated_event.description &
                                static_cast<std::uint32_t>(
                                    SysEpoll::Description::Error)) {
                                debug("Event with fd = " +
                                      std::to_string(
                                          associated_event.file_descriptor) +
                                      " must be closed");
                                Remove(associated_socket);
                        }
                        if (associated_event.description &
                            static_cast<std::uint32_t>(
                                SysEpoll::Description::Read)) {
                                debug(
                                    "Socket with fd = " +
                                    std::to_string(associated_socket.GetFD()) +
                                    " can be read from");
                                bool should_close = callback(associated_socket);
                                if (should_close)
                                        Remove(associated_socket);
                        }
                }
        }

        void AddNewConnections(const Sock &acceptor) noexcept {
                while (auto new_connection = acceptor.Accept()) {
                        new_connection.MakeNonBlocking();
                        debug("Will add a new connection with fd = " +
                              std::to_string(new_connection.GetFD()));
                        SocketWatcher<Sock>::Add(std::move(new_connection));
                }
        }
};
}

#endif
