#include <server/server.h>
#include <server/dispatcher.h>
#include <io/outputscheduler.h>
#include <io/schedulers/out.h>
#include <io/watchers/socket_watcher.h>
#include <http/parser.h>
#include <misc/storage.h>
#include <misc/debug.h>

#include <utility>
#include <algorithm>
#include <iostream>
#include <thread>
#include <future>
#include <regex>
#include <map>
#include <functional>
#include <cassert>

using namespace Web;

std::mutex Log::mLock;
std::string Log::_fn;

Server::Server(int port) : _port(port) {
        Log::Init("log_file.txt");
        Log::SetEnabled(false);
        Log::i("Started logging");
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

                //                IO::OutputScheduler &output_scheduler =
                //                    IO::OutputScheduler::get();
                //                std::thread
                //                output_thread(&IO::OutputScheduler::Run,
                //                                          std::ref(output_scheduler));
                //                output_thread.detach();

                IO::SocketWatcher<IO::Socket> watcher(std::move(master_socket));
                while (true) {
                        using namespace std::placeholders;
                        watcher.Run(
                            std::bind(&Dispatcher::Dispatch, &dispatcher_, _1));
                }

        } catch (std::exception &ex) {
                Log::e(std::string("Server error: ").append(ex.what()));
                auto msg = std::string(
                    "Server error. Please see the log file. Last exception: ");
                msg += ex.what();
                msg += "\n";
                throw std::runtime_error(msg);
        }
}
int Server::maxPending() const { return _maxPending; }

void Server::setMaxPending(int maxPending) { _maxPending = maxPending; }
