//
// Created by Vladimir on 8/2/2015.
//

#ifndef SOCKET_SOCKET_H
#define SOCKET_SOCKET_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <memory>
#include <unistd.h>
#include <vector>
#include <functional>

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
    bool was_shut_down() const;
    void close();

    template <typename T> T read_some() const {
        T result;
        auto available = available_read();
        result.resize(static_cast<std::size_t>(available));
        ssize_t readBytes = ::read(fd_, &result.front(), available);
        if (readBytes == 0)
            throw connection_closed_by_peer{fd_, this};
        if (readBytes != -1)
            result.resize(static_cast<std::size_t>(readBytes));
        return result;
    }

    template <typename T> std::size_t write_some(const T &data) const {
        auto written = ::send(fd_, static_cast<const void *>(data.data()), data.size(), MSG_NOSIGNAL);
        if (written <= 0) {
            if (!((errno == EAGAIN) || (errno == EWOULDBLOCK)))
                throw write_error{fd_, this};
            if (errno == ECONNRESET)
                throw connection_closed_by_peer{fd_, this};
            if (errno == EPIPE)
                throw connection_closed_by_peer{fd_, this};
        }
        return written == -1 ? 0 : written;
    }

    private:
    int fd_ = -1;
    bool connection_ = false;
    struct sockaddr_in address_;
    int port_;
};
}

#endif // SOCKET_SOCKET_H
