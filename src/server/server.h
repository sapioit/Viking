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
#ifndef SERVER_H
#define SERVER_H

#include <misc/settings.h>
#include <http/resolution.h>
#include <http/request.h>
#include <http/routeutility.h>
#include <vector>
#include <regex>

namespace web {
class server {
    class server_impl;
    server_impl *impl;

    public:
    server(int);
    ~server();
    server(const server &) = delete;
    server &operator=(const server &) = delete;
    server(server &&);
    server &operator=(server &&);
    void add_route(const http::method &method, const std::function<bool(const std::string &)> validator,
                   http_handler function);
    void add_route(const http::method &method, const std::regex &regex, http_handler function);
    void set_config(const configuration &);
    void init();
    void run(bool indefinitely = true);
    void freeze();
    std::string get_version() const noexcept;
    struct port_in_use {
        int port;
    };
};
}

#endif // SERVER_H
