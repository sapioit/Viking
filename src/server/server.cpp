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

Server::Server(int port) : port_(port) {}

Server &Server::operator=(Server &&other) {
    if (this != &other) {
        dispatcher_ = other.dispatcher_;
        other.dispatcher_ = {};
        port_ = other.port_;
    }
    return *this;
}

Server::Server(Server &&other) { *this = std::move(other); }

void Server::SetSettings(const Settings &s) {
    Storage::SetSettings(s);
    max_pending_ = s.max_connections;
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
        IO::Scheduler watcher(std::unique_ptr<IO::Socket>(make_socket(port_, max_pending_)),
                              [this](const IO::Socket *socket) { return dispatcher_.HandleConnection(socket); },
                              [this](ScheduleItem &schedule_item, std::unique_ptr<MemoryBuffer> &buf) {
                                  return dispatcher_.HandleBarrier(schedule_item, buf);
                              });
        while (true) {
            watcher.Run();
        }

    } catch (...) {
        std::rethrow_exception(std::current_exception());
    }
}
