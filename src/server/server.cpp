#include <io/schedulers/io_scheduler.h>
#include <http/dispatcher/dispatcher.h>
#include <server/server.h>
#include <misc/storage.h>
#include <misc/debug.h>
#include <sys/types.h>
#include <unistd.h>

#include <functional>
#include <signal.h>
#include <fstream>

using namespace Web;

IO::Socket *MakeSocket(int port, int max_pending);

class Server::ServerImpl {
    int port_;
    int max_pending_;
    bool stop_requested_;
    Dispatcher dispatcher_;
    IO::Scheduler scheduler_;

    inline void IgnoreSigpipe() { signal(SIGPIPE, SIG_IGN); }

    public:
    ServerImpl(int port) : port_(port), stop_requested_(false) {}

    inline void Initialize() {
        IgnoreSigpipe();
        debug("Pid = " + std::to_string(getpid()));
        if (auto sock = MakeSocket(port_, max_pending_)) {
            scheduler_ =
                IO::Scheduler(std::unique_ptr<IO::Socket>(sock),
                              [this](const IO::Socket *socket) { return dispatcher_.HandleConnection(socket); },
                              [this](ScheduleItem & schedule_item) -> auto {
                                  if (schedule_item.IsFrontAsync()) {
                                      AsyncBuffer<Http::Response> *async_buffer =
                                          static_cast<AsyncBuffer<Http::Response> *>(schedule_item.Front());
                                      if (async_buffer->IsReady())
                                          return dispatcher_.HandleBarrier(async_buffer);
                                  }
                                  return std::make_unique<MemoryBuffer>(std::vector<char>());
                              });
        } else
            throw Server::PortInUse{port_};
    }

    inline void Run(bool indefinitely) {
        if (!indefinitely)
            scheduler_.Run();
        else {
            stop_requested_ = false;
            while (!stop_requested_) {
                scheduler_.Run();
            }
        }
    }

    inline void Freeze() { stop_requested_ = true; }

    inline void AddRoute(const Http::Method &method, const std::string &uri_regex,
                  std::function<Http::Resolution(Http::Request)> function) {
        dispatcher_.AddRoute(std::make_pair(std::make_pair(method, uri_regex), function));
    }

    inline void SetSettings(const Settings &s) {
        Storage::SetSettings(s);
        max_pending_ = s.max_connections;
    }
};

IO::Socket *MakeSocket(int port, int max_pending) {
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

Server::Server(int port) { impl = new ServerImpl(port); }

Server::~Server() { delete impl; }

Server &Server::operator=(Server &&other) {
    if (this != &other) {
        impl = other.impl;
        other.impl = nullptr;
    }
    return *this;
}

Server::Server(Server &&other) { *this = std::move(other); }

void Server::AddRoute(const Http::Method &method, const std::string &uri_regex,
                      std::function<Http::Resolution(Http::Request)> function) {
    impl->AddRoute(method, uri_regex, function);
}

void Server::SetSettings(const Settings &s) { impl->SetSettings(s); }

void Server::Initialize() { impl->Initialize(); }

void Server::Run(bool indefinitely) { impl->Run(indefinitely); }

void Server::Freeze() { impl->Freeze(); }
