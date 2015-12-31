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
#include <io/buffers/mem_buffer.h>
#include <http/engine.h>
#include <http/routeutility.h>
#include <http/util.h>
#include <http/resolution.h>
#include <http/response_serializer.h>
#include <cache/file_descriptor.h>
#include <cache/resource_cache.h>
#include <server/server.h>
#include <misc/storage.h>
#include <misc/debug.h>
#include <misc/resource.h>
#include <http/response.h>
#include <sys/types.h>
#include <unistd.h>

#include <functional>
#include <future>
#include <signal.h>
#include <fstream>
#include <memory>

using namespace Web;
using namespace Cache;
using namespace IO;
using namespace Http;
static ResponseSerializer serializer;
IO::Socket *MakeSocket(int port, int max_pending);

class Server::ServerImpl {
    int port_;
    int max_pending_;
    bool stop_requested_;
    IO::Scheduler scheduler_;
    RouteUtility::RouteMap routes;

    template <typename T> void PassToUser(T begin, T end) {
        std::for_each(begin, end, [this](auto channel) {
            Context &ctx = (*static_cast<Context *>(channel->data));
            auto http_request = ctx.GetRequest();
            auto handler = RouteUtility::GetUserHandler(http_request, this->routes);
            Http::Resolution resolution = handler(http_request);
            if (resolution.GetType() == Http::Resolution::Type::Sync) {
                ScheduleItem item{serializer(resolution.GetResponse()), resolution.GetResponse().GetKeepAlive()};
                channel->queue.PutBack(std::move(item));
            } else {
                ScheduleItem item{std::make_unique<AsyncBuffer<Http::Response>>(std::move(resolution.GetFuture()))};
                channel->queue.PutAfterFirstIntact(std::move(item));
            }
        });
    }

    void ScheduleResource(const Http::Request &request, Channel *channel) const noexcept {
        auto full_path = Storage::GetSettings().root_path + request.url;
        try {
            if (ShouldCopyInMemory(full_path)) {
                if (auto resource = ResourceCache::Aquire(full_path)) {
                    Http::Response response{std::move(resource)};
                    response.Set(Http::Header::Fields::Connection, ShouldKeepAlive(request) ? "Keep-Alive" : "Close");
                    ScheduleItem item = {serializer(response), response.GetKeepAlive()};
                    channel->queue.PutBack(std::move(item));
                } else {
                    throw Http::StatusCode::NotFound;
                }
            } else {
                auto unix_file = std::make_unique<UnixFile>(full_path, Cache::FileDescriptor::Aquire,
                                                            Cache::FileDescriptor::Release);
                Http::Response http_response;
                http_response.SetFile(unix_file.get());
                http_response.Set(Http::Header::Fields::Content_Type, Http::Util::GetMimeType(full_path));
                http_response.Set(Http::Header::Fields::Content_Length, std::to_string(unix_file->size));
                http_response.Set(Http::Header::Fields::Connection, ShouldKeepAlive(request) ? "Keep-Alive" : "Close");
                ScheduleItem item;
                item.PutBack(std::make_unique<MemoryBuffer>(serializer.MakeHeader(http_response)));
                item.PutBack(std::move(unix_file));
                item.PutBack(std::make_unique<MemoryBuffer>(serializer.MakeEnding(http_response)));
                item.SetKeepFileOpen(http_response.GetKeepAlive());
                channel->queue.PutBack(std::move(item));
            }
        } catch (...) {
            ScheduleItem item{serializer({Http::StatusCode::NotFound})};
            channel->queue.PutBack(std::move(item));
        }
    }

    template <typename T> void ProcessRequestsAsync(T begin, T end) {
        auto len = end - begin;
        if (len < 10) {
            for (auto it = begin; it != end; ++it) {
                Channel *ch = static_cast<Channel *>(*it);
                Context *ctx = static_cast<Context *>(ch->data);
                auto http_request = ctx->GetRequest();
                if (Http::Util::IsResource(http_request)) {
                    ScheduleResource(http_request, ch);
                } else {
                    // TODO handle internally
                }
            }
        } else {
            auto mid = begin + len / 2;
            auto handle = std::async(std::launch::async, [this, begin, mid]() { ProcessRequestsAsync(begin, mid); });
            ProcessRequestsAsync(mid, end);
            handle.get();
        }
    }

    void HandleConnections(std::vector<Channel *> channels) {
        std::vector<Channel*> to_remove;
        for (auto channel : channels) {
            if (channel->data == nullptr) {
                channel->data = new Context(channel->socket.get());
                channel->on_destroy = std::bind(&ServerImpl::DeleteContext, this, channel->data);
            }
            Context &ctx = (*static_cast<Context *>(channel->data));
            try {
                ctx();
            } catch (const Socket::ConnectionClosedByPeer &) {
                scheduler_.Remove(channel);
                to_remove.push_back(channel);
                continue;
            }
        }

        for(auto ch : to_remove)
            channels.erase(std::remove(channels.begin(), channels.end(), ch));

        auto first_automatic = std::partition(channels.begin(), channels.end(), [this](Channel *channel) {
            Context &ctx = (*static_cast<Context *>(channel->data));
            return HasUserHandler(ctx.GetRequest());
        });
        PassToUser(channels.begin(), first_automatic);
        ProcessRequestsAsync(first_automatic, channels.end());
    }

    bool HasUserHandler(const Http::Request &r) {
        bool is_passable = Http::Util::IsPassable(r);
        bool is_resource = Http::Util::IsResource(r);
        bool has_handler = RouteUtility::GetUserHandler(r, routes) != nullptr;

        return is_passable && !is_resource && has_handler;
    }

    bool ShouldKeepAlive(const Http::Request &r) const noexcept {
        auto it = r.header.fields.find(Http::Header::Fields::Connection);
        if (it != r.header.fields.end() && it->second == "Keep-Alive")
            return true;
        return false;
    }

    bool ShouldCopyInMemory(const fs::path &resource_path) const {
        try {
            auto page_size = static_cast<std::size_t>(getpagesize());
            auto file_size = fs::file_size(resource_path);
            return file_size <= page_size;
        } catch (...) {
            throw Http::StatusCode::NotFound;
        }
    }

    inline void DeleteContext(void *ctx) { delete static_cast<Context *>(ctx); }

    inline void AddRoute(RouteUtility::Route r) noexcept { routes.push_back(r); }

    inline void IgnoreSigpipe() { signal(SIGPIPE, SIG_IGN); }

    std::unique_ptr<MemoryBuffer> HandleBarrier(ScheduleItem &schedule_item) noexcept {

        if (schedule_item.IsFrontAsync()) {
            AsyncBuffer<Http::Response> *async_buffer =
                static_cast<AsyncBuffer<Http::Response> *>(schedule_item.Front());
            if (async_buffer->IsReady()) {
                auto http_response = async_buffer->future.get();
                return std::make_unique<MemoryBuffer>(serializer(http_response));
            }
        }
        return std::make_unique<MemoryBuffer>(std::vector<char>());
    }

    public:
    ServerImpl(int port) : port_(port), stop_requested_(false) {}

    inline void Initialize() {
        IgnoreSigpipe();
        debug("Pid = " + std::to_string(getpid()));
        if (auto sock = MakeSocket(port_, max_pending_)) {
            using namespace std::placeholders;
            scheduler_ =
                IO::Scheduler(std::unique_ptr<IO::Socket>(sock), std::bind(&ServerImpl::HandleConnections, this, _1),
                              std::bind(&ServerImpl::HandleBarrier, this, _1));
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
        AddRoute(std::make_pair(std::make_pair(method, validator), function));
    }

    inline void AddRoute(const Http::Method &method, const std::regex &regex,
                         std::function<Http::Resolution(Http::Request)> function) {

        auto reg = [regex](auto string) {
            try {
                return std::regex_match(string, regex);
            } catch (const std::regex_error &) {
                return false;
            }
        };
        AddRoute(std::make_pair(std::make_pair(method, reg), function));
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

std::string Server::GetVersion() const noexcept { return "0.7.8"; }
