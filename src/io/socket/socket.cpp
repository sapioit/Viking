//
// Created by Vladimir on 8/2/2015.
//

#include <io/socket/socket.h>
#include <misc/log.h>
#include <misc/debug.h>

#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <system_error>
#include <assert.h>

using namespace IO;

Socket::Socket(int port) : port_(port) {
    if ((fd_ = ::socket(AF_INET, SOCK_STREAM, 0)) == -1)
        throw std::runtime_error("Could not create socket");
    int opt = 1;
    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error("Setsockopt error");
    address_.sin_family = AF_INET;
    address_.sin_addr.s_addr = INADDR_ANY;
    address_.sin_port = htons(port);
}

Socket::Socket(int fd, int port) : fd_(fd), port_(port), connection_(true) {}

Socket::Socket(Socket &&other) : fd_(-1) { *this = std::move(other); }

Socket &Socket::operator=(Socket &&other) {
    if (this != &other) {
        Close();
        fd_ = other.fd_;
        port_ = other.port_;
        address_ = other.address_;
        connection_ = other.connection_;
        other.fd_ = -1;
    }
    return *this;
}

void Socket::Bind() const {
    if (::bind(fd_, reinterpret_cast<const struct sockaddr *>(&address_), sizeof(address_)) == -1) {
        if (errno == EADDRINUSE)
            throw std::runtime_error("Port " + std::to_string(port_) + " is already in use (errno = " +
                                     std::to_string(errno) + ")\n");
        throw std::runtime_error("Bind failed, errno = " + std::to_string(errno));
    }
}

void Socket::Listen(int pending_max) const {
    int listen_result = ::listen(fd_, pending_max);
    if (listen_result < 0)
        throw std::runtime_error("Listen failed");
}

void Socket::MakeNonBlocking() const {
    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error("Could not get file descriptor flags");

    flags |= O_NONBLOCK;
    if (fcntl(fd_, F_SETFL, flags) == -1)
        throw std::runtime_error("Could not set the non-blocking flag "
                                 "for the file descriptor");
}
int Socket::AvailableToRead() const {
    long count;
    if (-1 == ioctl(fd_, FIONREAD, &count))
        throw InternalSocketError{fd_, this};
    return count;
}
Socket Socket::Accept() const {
    struct sockaddr in_addr;
    socklen_t in_len;
    in_len = sizeof(in_addr);
    return Socket(::accept(fd_, &in_addr, &in_len), port_);
}

int Socket::GetFD() const { return fd_; }

bool Socket::IsAcceptor() const { return (!connection_); }

void Socket::Close() {
    if (fd_ != -1) {
        debug("Closing socket with fd = " + std::to_string(fd_));
        ::close(fd_);
        fd_ = -1;
    }
}

Socket::~Socket() { Close(); }

Socket Socket::start_socket(int port, int maxConnections) {
    try {
        Socket socket(port);
        socket.Bind();
        socket.MakeNonBlocking();
        socket.Listen(maxConnections);
        return socket;
    } catch (std::exception &ex) {
        throw;
    }
}

bool Socket::WasShutDown() const {
    char a;
    return (::recv(fd_, &a, 1, MSG_PEEK) == 0);
}

bool Socket::operator<(const Socket &other) const { return fd_ < other.fd_; }
bool Socket::operator==(const Socket &other) const { return fd_ == other.fd_; }

Socket::operator bool() const { return fd_ != -1; }
