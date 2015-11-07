#ifndef SOCKET_WATCHER_H
#define SOCKET_WATCHER_H

#include <io/watchers/file_watcher.h>
#include <io/socket/socket.h>
#include <io/watchers/sys_epoll.h>
#include <misc/debug.h>
#include <stdexcept>
#include <algorithm>

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
                try {
                        SysEpoll::Remove(socket.GetFD());
                        FileWatcher<Sock>::Remove(socket);
                } catch (const typename FileWatcher<Sock>::FileNotFound &ex) {
                        // WTF?
                        // debug("FileWatcher could not remove the file with fd
                        // = " + std::to_string(ex.fd));
                }
        }

        virtual void
        Run(std::function<void(std::vector<std::reference_wrapper<Sock>>)>
                callback) {
                const auto events = std::move(
                    SysEpoll::Wait(FileWatcher<Sock>::watched_files_.size()));

                std::vector<std::reference_wrapper<Sock>> active_sockets;

                for (const auto &event : events) {
                        auto sock_it = std::find_if(
                            FileWatcher<Sock>::watched_files_.begin(),
                            FileWatcher<Sock>::watched_files_.end(),
                            [this, &event](const Sock &socket) {
                                    if (event.file_descriptor ==
                                        socket.GetFD()) {
                                            debug(
                                                "Matched the event with fd = " +
                                                std::to_string(
                                                    event.file_descriptor) +
                                                " with an active socket");
                                            return true;
                                    }
                                    return false;
                            });
                        if (sock_it !=
                            FileWatcher<Sock>::watched_files_.end()) {

                                auto &associated_socket = (*sock_it);

                                if (associated_socket.IsAcceptor()) {
                                        debug("The master socket is active");
                                        AddNewConnections(associated_socket);
                                        continue;
                                }

                                if (event.description &
                                        static_cast<std::uint32_t>(
                                            SysEpoll::Description::
                                                Termination) ||
                                    event.description &
                                        static_cast<std::uint32_t>(
                                            SysEpoll::Description::Error)) {
                                        debug("Event with fd = " +
                                              std::to_string(
                                                  event.file_descriptor) +
                                              " must be closed");
                                        Remove(associated_socket);
                                }
                                if (event.description &
                                    static_cast<std::uint32_t>(
                                        SysEpoll::Description::Read)) {
                                        debug("Socket with fd = " +
                                              std::to_string(
                                                  associated_socket.GetFD()) +
                                              " can be read from");
                                        active_sockets.emplace_back(
                                            associated_socket);
                                }
                        }

                        debug("We have " +
                              std::to_string(active_sockets.size()) +
                              " active sockets out of " +
                              std::to_string(
                                  FileWatcher<Sock>::watched_files_.size()));

                        callback(active_sockets);
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
