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

void Server::AddRoute(Http::Method method, const std::string &uri_regex,
                      std::function<Http::Resolution(Http::Request)> function) {
    dispatcher_.AddRoute(std::make_pair(std::make_pair(method, uri_regex), function));
}

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
        if (auto sock = make_socket(port_, max_pending_)) {
            IO::Scheduler watcher(std::unique_ptr<IO::Socket>(sock),
                                  [this](const IO::Socket *socket) { return dispatcher_.HandleConnection(socket); },
                                  [this](ScheduleItem & schedule_item) -> auto {
                                      if (schedule_item.IsFrontAsync()) {
                                          AsyncBuffer<Http::Response> *async_buffer =
                                              static_cast<AsyncBuffer<Http::Response> *>(schedule_item.Front());
                                          if (async_buffer->IsReady()) {
                                              return dispatcher_.HandleBarrier(async_buffer);
                                          }
                                      }
                                      return std::make_unique<MemoryBuffer>(std::vector<char>());
                                  });
            while (true) {
                watcher.Run();
            }
        } else
            throw IO::Socket::PortInUse{};

    } catch (...) {
        std::rethrow_exception(std::current_exception());
    }
}
