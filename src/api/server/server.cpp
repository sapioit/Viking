#include <server/server.h>
#include <server/dispatcher.h>
#include <io/outputscheduler.h>
#include <io/watchers/socket_watcher.h>
#include <http/parser.h>
#include <misc/storage.h>
#include <misc/debug.h>

#include <utility>
#include <algorithm>
#include <functional>
#include <iostream>
#include <thread>
#include <future>
#include <regex>
#include <map>
#include <cassert>

using namespace Web;

std::mutex Log::mLock;
std::string Log::_fn;
Server::Server(int port) : _port(port) {
        Log::Init("log_file.txt");
        Log::SetEnabled(false);
        Log::i("Started logging");
        setSettings({});
}

void Server::setSettings(const Settings &s) {
        Storage::setSettings(s);
        _maxPending = s.max_connections;
}

void Server::run() {
        if (_port == -1)
                throw std::runtime_error("Port number not set!");
        try {
                auto master_socket =
                    IO::Socket::start_socket(_port, _maxPending);
                debug("Master socket has fd = " +
                      std::to_string(master_socket.GetFD()));
                // IO::Watcher _master_listener(_masterSocket, _maxPending);

                IO::OutputScheduler &output_scheduler =
                    IO::OutputScheduler::get();
                std::thread output_thread(&IO::OutputScheduler::Run,
                                          std::ref(output_scheduler));

                output_thread.detach();

                IO::SocketWatcher<IO::Socket> watcher(std::move(master_socket));

                //                auto watcher_callbacks = [&](
                //                    std::vector<const IO::Socket *> sockets) {
                //                        debug("Will dispatch " +
                //                              std::to_string(sockets.size()) +
                //                              " connections");
                //                        for (auto &sock : sockets) {
                //                                auto should_close =
                //                                Dispatcher::Dispatch(*sock);
                //                                if (should_close)
                //                                        watcher.Remove(*sock);
                //                        }
                //                };

                while (true) {
                        watcher.Run(Dispatcher::Dispatch);
                }

        } catch (std::exception &ex) {
                Log::e(std::string("Server error: ").append(ex.what()));
                auto msg = std::string(
                    "Server error. Please see the log file. Last exception: ");
                msg += ex.what();
                throw std::runtime_error(msg);
        }
}
int Server::maxPending() const { return _maxPending; }

void Server::setMaxPending(int maxPending) { _maxPending = maxPending; }
