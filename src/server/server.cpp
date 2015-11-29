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

Server::Server(int port) : _port(port) {}

void Server::SetSettings(const Settings &s) {
    Storage::SetSettings(s);
    _maxPending = s.max_connections;
}

IO::Socket *make_socket(int port, int max_pending, bool = false) {
    try {
        auto sock = new IO::Socket(port);
        sock->Bind();
        sock->MakeNonBlocking();
        sock->Listen(max_pending);
        return sock;
    } catch (const IO::Socket::PortInUse &) {
        return nullptr;
    }
}

void Server::Run() {
    signal(SIGPIPE, SIG_IGN);
    debug("Pid = " + std::to_string(getpid()));
    try {
        IO::Scheduler watcher(std::move(std::unique_ptr<IO::Socket>(make_socket(_port, _maxPending))),
                              Dispatcher::HandleConnection, Dispatcher::HandleBarrier);
        while (true) {
            watcher.Run();
        }

    } catch (...) {
        std::rethrow_exception(std::current_exception());
    }
}
int Server::GetMaxPending() const { return _maxPending; }

void Server::SetMaxPending(int maxPending) { _maxPending = maxPending; }
