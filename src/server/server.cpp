#include <io/schedulers/io_scheduler.h>
#include <server/server.h>
#include <misc/storage.h>
#include <misc/debug.h>
#include <sys/types.h>
#include <unistd.h>

#include <functional>
#include <signal.h>
#include <fstream>

using namespace Web;

std::mutex Log::mLock;
std::string Log::_fn;

Server::Server(int port) : _port(port) {}

void Server::SetSettings(const Settings &s) {
    Storage::SetSettings(s);
    _maxPending = s.max_connections;
}

IO::Socket *make_socket(int port, int max_pending, bool = false) {
    auto sock = new IO::Socket(port);
    sock->Bind();
    sock->MakeNonBlocking();
    sock->Listen(max_pending);
    return sock;
}

void Server::Run() {
    signal(SIGPIPE, SIG_IGN);
    debug("Pid = " + std::to_string(getpid()));
    if (_port == -1)
        throw std::runtime_error("Port number not set!");
    try {
        auto master_socket = std::unique_ptr<IO::Socket>(make_socket(_port, _maxPending));

        IO::Scheduler watcher(std::move(master_socket), Dispatcher::HandleConnection);
        while (true) {
            watcher.Run();
        }

    } catch (std::exception &ex) {
        Log::e(std::string("Server error: ").append(ex.what()));
        auto msg = std::string("Server error. Please see the log file. Last exception: ");
        msg += ex.what();
        msg += "\n";
        throw std::runtime_error(msg);
    }
}
int Server::GetMaxPending() const { return _maxPending; }

void Server::SetMaxPending(int maxPending) { _maxPending = maxPending; }
