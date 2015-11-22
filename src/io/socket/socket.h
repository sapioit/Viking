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

namespace IO {
class Socket {
    public:
    struct AcceptError {
        int fd;
        const Socket *ptr;
    };
    struct WriteError {
        int fd;
        const Socket *ptr;
    };
    struct ConnectionClosedByPeer {
        int fd;
        const Socket *ptr;
    };
    struct InternalSocketError {
        int fd;
        const Socket *ptr;
    };

    Socket(int);
    Socket(int port, int);
    Socket(const Socket &) = delete;
    Socket(Socket &&other);
    Socket &operator=(Socket &&other);
    Socket &operator=(const Socket &) = delete;
    bool operator<(const Socket &) const;
    bool operator==(const Socket &) const;
    operator bool() const;
    virtual ~Socket();
    Socket Accept() const;
    int GetFD() const;
    bool IsAcceptor() const;
    void Bind() const;
    void MakeNonBlocking() const;
    void Listen(int pending_max) const;
    int AvailableToRead() const;
    bool WasShutDown() const;
    void Close();

    static Socket start_socket(int port, int maxConnections);

    template <typename T> T ReadSome() const {
        T result;
        auto available = AvailableToRead();
        result.resize(static_cast<std::size_t>(available));
        ssize_t readBytes = ::read(fd_, &result.front(), available);
        if (readBytes == 0)
            throw ConnectionClosedByPeer{fd_, this};
        if (readBytes != -1)
            result.resize(static_cast<std::size_t>(readBytes));
        return result;
    }

    template <typename T> std::size_t WriteSome(const T &data) const {
        auto written = ::send(fd_, static_cast<const void *>(data.data()), data.size(), MSG_NOSIGNAL);
        if (written <= 0) {
            if (!((errno == EAGAIN) || (errno == EWOULDBLOCK)))
                throw WriteError{fd_, this};
            if (errno == ECONNRESET)
                throw ConnectionClosedByPeer{fd_, this};
            if (errno == EPIPE)
                throw ConnectionClosedByPeer{fd_, this};
        }
        return written == -1 ? 0 : written;
    }

    private:
    int fd_ = -1;
    int port_;
    bool connection_ = false;
    struct sockaddr_in address_;
};
typedef std::reference_wrapper<Socket> SocketRef;
}

#endif // SOCKET_SOCKET_H
