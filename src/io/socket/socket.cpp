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

#include <io/socket/socket.h>
#include <misc/debug.h>

#include <assert.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <system_error>

using namespace io;

tcp_socket::tcp_socket(int port) : connection_(false), port_(port) {
    if ((fd_ = ::socket(AF_INET, SOCK_STREAM, 0)) == -1)
        throw std::runtime_error("Could not create socket");
    int opt = 1;
    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error("Setsockopt error");
    address_.sin_family = AF_INET;
    address_.sin_addr.s_addr = INADDR_ANY;
    address_.sin_port = htons(port);
}

tcp_socket::tcp_socket(int fd, int port) : fd_(fd), connection_(true), port_(port) {}

tcp_socket::tcp_socket(tcp_socket &&other) : fd_(-1) { *this = std::move(other); }

tcp_socket &tcp_socket::operator=(tcp_socket &&other) {
    if (this != &other) {
        close();
        fd_ = other.fd_;
        port_ = other.port_;
        address_ = other.address_;
        connection_ = other.connection_;
        other.fd_ = -1;
    }
    return *this;
}

void tcp_socket::bind() const {
    if (::bind(fd_, reinterpret_cast<const struct sockaddr *>(&address_), sizeof(address_)) == -1)
        if (errno == EADDRINUSE)
            throw port_in_use{port_};
}

void tcp_socket::listen(int pending_max) const {
    int listen_result = ::listen(fd_, pending_max);
    if (listen_result < 0)
        throw std::runtime_error("Listen failed");
}

void tcp_socket::make_non_blocking() const {
    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error("Could not get file descriptor flags");

    flags |= O_NONBLOCK;
    if (fcntl(fd_, F_SETFL, flags) == -1)
        throw std::runtime_error("Could not set the non-blocking flag "
                                 "for the file descriptor");
}
int tcp_socket::available_read() const {
    int count;
    if (-1 == ioctl(fd_, FIONREAD, &count)) {
        debug("ioctl failed, errno " + std::to_string(errno));
        throw internal_error{fd_, this};
    }
    return count;
}
std::unique_ptr<tcp_socket> tcp_socket::accept() const {
    struct sockaddr in_addr;
    socklen_t in_len;
    in_len = sizeof(in_addr);
    return std::make_unique<tcp_socket>(::accept(fd_, &in_addr, &in_len), port_);
}

bool tcp_socket::is_acceptor() const { return (!connection_); }

void tcp_socket::close() {
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
}

tcp_socket::~tcp_socket() { close(); }

bool tcp_socket::operator<(const tcp_socket &other) const { return fd_ < other.fd_; }
bool tcp_socket::operator==(const tcp_socket &other) const { return fd_ == other.fd_; }

tcp_socket::operator bool() const { return fd_ != -1; }
