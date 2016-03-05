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

using namespace web;
using namespace io;
tcp_socket *make_socket(int port, int max_pending);

class server::server_impl {
    int m_port;
    int m_max_pending;
    bool m_stop_requested;
    dispatcher m_dispatcher;
    io::scheduler m_scheduler;

    inline void ignore_sigpipe() { signal(SIGPIPE, SIG_IGN); }

    public:
    server_impl(int port) : m_port(port), m_stop_requested(false) {}

    inline void init() {
        ignore_sigpipe();
        debug("Pid = " + std::to_string(getpid()));
        if (auto sock = make_socket(m_port, m_max_pending)) {
            m_scheduler = io::scheduler(std::unique_ptr<io::tcp_socket>(sock),
                                        [this](io::channel *ch) {
                                            try {
                                                (void)ch;
                                                // THIS FUNCTION WILL BE REMOVED
                                                std::terminate();
                                                // m_dispatcher.handle_connection(ch);
                                            } catch (...) {
                                                throw;
                                            }
                                        },
                                        [this](auto vec, auto fn) { m_dispatcher.handle_connections(vec, fn); },
                                        [this](schedule_item & schedule_item) -> auto {
                                            if (schedule_item.is_front_async()) {
                                                async_buffer<http::response> *buffer =
                                                    static_cast<async_buffer<http::response> *>(schedule_item.front());
                                                if (buffer->is_ready())
                                                    return m_dispatcher.handle_barrier(buffer);
                                            }
                                            return std::make_unique<memory_buffer>(std::vector<char>{});
                                        },
                                        [this](io::channel *ch) { m_dispatcher.will_remove(ch); });
        } else {
            throw server::port_in_use{m_port};
        }
    }

    inline void run(bool indefinitely) {
        if (!indefinitely)
            m_scheduler.run();
        else {
            m_stop_requested = false;
            while (!m_stop_requested) {
                m_scheduler.run();
            }
        }
    }

    inline void freeze() { m_stop_requested = true; }

    inline void add_route(const http::method &method, std::function<bool(const std::string &)> validator,
                          std::function<http::resolution(http::request)> function) {
        m_dispatcher.add_route(std::make_pair(std::make_pair(method, validator), function));
    }

    inline void add_route(const http::method &method, const std::regex &regex,
                          std::function<http::resolution(http::request)> function) {

        auto ptr = [regex](auto string) {
            try {
                return std::regex_match(string, regex);
            } catch (const std::regex_error &) {
                return false;
            }
        };
        m_dispatcher.add_route(std::make_pair(std::make_pair(method, ptr), function));
    }

    inline void set_config(const configuration &s) {
        storage::set_config(s);
        m_max_pending = s.max_connections;
    }
};

io::tcp_socket *make_socket(int port, int max_pending) {
    try {
        auto sock = new io::tcp_socket(port);
        sock->bind();
        sock->make_non_blocking();
        sock->listen(max_pending);
        return sock;
    } catch (const io::tcp_socket::port_in_use &) {
        return nullptr;
    }
}

server::server(int port) { impl = new server_impl(port); }

server::~server() { delete impl; }

server &server::operator=(server &&other) {
    if (this != &other) {
        impl = other.impl;
        other.impl = nullptr;
    }
    return *this;
}

server::server(server &&other) { *this = std::move(other); }

void server::add_route(const http::method &method, std::function<bool(const std::string &)> validator,
                       std::function<http::resolution(http::request)> function) {
    impl->add_route(method, validator, function);
}

void server::add_route(const http::method &method, const std::regex &regex,
                       std::function<http::resolution(http::request)> function) {
    impl->add_route(method, regex, function);
}

void server::set_config(const configuration &s) { impl->set_config(s); }

void server::init() { impl->init(); }

void server::run(bool indefinitely) { impl->run(indefinitely); }

void server::freeze() { impl->freeze(); }

std::string server::get_version() const noexcept { return "0.7.9"; }
