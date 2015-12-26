/*
Copyright (C) 2015 Voinea Constantin Vladimir

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/
#include <io/schedulers/io_scheduler.h>
#include <io/schedulers/channel.h>
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
            scheduler_ = IO::Scheduler(std::unique_ptr<IO::Socket>(sock),
                                       [this](const IO::Channel *ch) {
                                           try {
                                               return dispatcher_.HandleConnection(ch);
                                           } catch (...) {
                                               throw;
                                           }
                                       },
                                       [this](ScheduleItem & schedule_item) -> auto {
                                           if (schedule_item.IsFrontAsync()) {
                                               AsyncBuffer<Http::Response> *async_buffer =
                                                   static_cast<AsyncBuffer<Http::Response> *>(schedule_item.Front());
                                               if (async_buffer->IsReady())
                                                   return dispatcher_.HandleBarrier(async_buffer);
                                           }
                                           return std::make_unique<MemoryBuffer>(std::vector<char>());
                                       },
                                       [this](const IO::Channel *ch) { dispatcher_.WillRemove(ch); });
        } else {
            throw Server::PortInUse{port_};
        }
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

    inline void AddRoute(const Http::Method &method, std::function<bool(const std::string &)> validator,
                         std::function<Http::Resolution(Http::Request)> function) {
        dispatcher_.AddRoute(std::make_pair(std::make_pair(method, validator), function));
    }

    inline void AddRoute(const Http::Method &method, const std::regex &regex,
                         std::function<Http::Resolution(Http::Request)> function) {

        auto ptr = [regex](auto string) {
            try {
                return std::regex_match(string, regex);
            } catch (const std::regex_error &) {
                return false;
            }
        };
        dispatcher_.AddRoute(std::make_pair(std::make_pair(method, ptr), function));
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

void Server::AddRoute(const Http::Method &method, std::function<bool(const std::string &)> validator,
                      std::function<Http::Resolution(Http::Request)> function) {
    impl->AddRoute(method, validator, function);
}

void Server::AddRoute(const Http::Method &method, const std::regex &regex,
                      std::function<Http::Resolution(Http::Request)> function) {
    impl->AddRoute(method, regex, function);
}

void Server::SetSettings(const Settings &s) { impl->SetSettings(s); }

void Server::Initialize() { impl->Initialize(); }

void Server::Run(bool indefinitely) { impl->Run(indefinitely); }

void Server::Freeze() { impl->Freeze(); }
