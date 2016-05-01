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
#ifndef SSL_SOCKET_H
#define SSL_SOCKET_H

#include "socket.h"
#include <ssl/s2n.h>

namespace io {
class ssl_socket : public tcp_socket {
    ssl_socket(int fd, int port);

    public:
    ssl_socket(int port);
    ssl_socket(const ssl_socket &) = delete;
    ssl_socket(ssl_socket &&other);
    ssl_socket &operator=(ssl_socket &&other);
    ssl_socket &operator=(const ssl_socket &) = delete;
    virtual ~ssl_socket();
    virtual std::unique_ptr<tcp_socket> accept() const override;
    virtual std::size_t write(const char *data, std::size_t, error_code &) const noexcept override;
    virtual std::size_t read(char *const, std::size_t, error_code &) const noexcept override;
    virtual std::string read(error_code &) const noexcept override;

    private:
    struct s2n_connection *ssl_connection;
    struct s2n_config *config;
};
};

#endif // SSL_SOCKET_H
