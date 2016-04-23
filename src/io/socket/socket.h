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
#ifndef SOCKET_SOCKET_H
#define SOCKET_SOCKET_H

#include <arpa/inet.h>
#include <functional>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace io {
class tcp_socket {
    public:
    struct accept_error {
        int fd;
        const tcp_socket *ptr;
    };
    struct write_error {
        int fd;
        const tcp_socket *ptr;
    };
    struct connection_closed_by_peer {
        int fd;
        const tcp_socket *ptr;
    };
    struct internal_error {
        int fd;
        const tcp_socket *ptr;
    };
    struct port_in_use {
        int port;
    };

    tcp_socket(int);
    tcp_socket(int port, int);
    tcp_socket(const tcp_socket &) = delete;
    tcp_socket(tcp_socket &&other);
    tcp_socket &operator=(tcp_socket &&other);
    tcp_socket &operator=(const tcp_socket &) = delete;
    bool operator<(const tcp_socket &) const;
    bool operator==(const tcp_socket &) const;
    operator bool() const;
    virtual ~tcp_socket();
    std::unique_ptr<tcp_socket> accept() const;
    inline int get_fd() const { return fd_; }
    bool is_acceptor() const;
    void bind() const;
    void make_non_blocking() const;
    void listen(int pending_max) const;
    int available_read() const;
    void close();

    template <typename T> T read_some() const {
        T result;

        static std::size_t max_read = 500;

        std::size_t bytes_read_total = 0;
        ssize_t bytes_read_loop = 0;
        do {
            auto old_size = result.size();
            result.resize(old_size + static_cast<std::size_t>(max_read));
            bytes_read_loop = ::read(fd_, &result.front() + old_size, max_read);
            bytes_read_total += bytes_read_loop > 0 ? bytes_read_loop : 0;
            result.resize(bytes_read_total);
            if (bytes_read_loop > 0)
                max_read = std::max(max_read, static_cast<std::size_t>(bytes_read_loop));
        } while (bytes_read_loop > 0);

        if (bytes_read_total == 0)
            throw connection_closed_by_peer{fd_, this};
        if (bytes_read_loop == -1)
            result.resize(static_cast<std::size_t>(bytes_read_total));
        return result;
    }

    template <typename T> std::size_t write_some(const T &data) const {

        auto total_to_write = data.size();
        std::size_t bytes_written_total = 0;
        ssize_t bytes_written_loop = 0;

        do {
            std::uint64_t flags = MSG_NOSIGNAL;
            auto left_to_write = total_to_write - bytes_written_total;
            static unsigned int page_size = getpagesize();
            if (left_to_write >= page_size / 2)
                flags |= MSG_MORE;
            bytes_written_loop =
                ::send(fd_, static_cast<const void *>(data.data() + bytes_written_total), left_to_write, flags);
            if (bytes_written_loop > 0)
                bytes_written_total += bytes_written_loop;
        } while (bytes_written_loop > 0 && bytes_written_total < total_to_write);

        if (bytes_written_loop == -1) {
            switch (errno) {
            case EWOULDBLOCK:
                return bytes_written_total;
            case EPIPE:
            case ECONNRESET:
                throw connection_closed_by_peer{fd_, this};
            default:
                break;
            }
        }
        return bytes_written_total;
    }

    private:
    int fd_ = -1;
    bool connection_ = false;
    struct sockaddr_in address_;
    int port_;
};
}

#endif // SOCKET_SOCKET_H
