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
                AcceptError(int sockfd) : fd(sockfd) {}
        };
        struct connection_closed_by_peer {};
        int get_fd() const;
        bool is_blocking() const;
        Socket(int port, bool connection);
        Socket(const Socket &) = delete;
        Socket(Socket &&other) {
                _fd = other._fd;
                other._fd = -1;
                _port = other._port;
                _reads = other._reads;
                _blocking = other._blocking;
                _address = other._address;
                _connection = other._connection;
        }
        Socket &operator=(Socket &&other) {
                if (this != &other) {
                        _fd = other._fd;
                        other._fd = -1;
                        _port = other._port;
                        _reads = other._reads;
                        _blocking = other._blocking;
                        _address = other._address;
                        _connection = other._connection;
                }
                return *this;
        }
        Socket &operator=(const Socket &) = delete;
        Socket *Duplicate() const;
        virtual ~Socket();
        static Socket start_socket(int port, int maxConnections);
        void Close();
        void Bind();
        void MakeNonBlocking();
        void Listen(int pending_max);
        Socket Accept() const;
        Socket(std::uint16_t fd);

        template <class T> T Read(std::size_t size = 0);

        std::string ReadUntil(const std::string &U, bool peek = false);
        ssize_t Write(const char *, std::size_t);
        ssize_t Write(const std::vector<char> &);
        ssize_t Write(const std::string &);
        bool WasShutDown();

        bool operator<(const Socket &) const;
        bool operator==(const Socket &) const;
        operator bool() const;
        std::uint64_t getReads() const;

        bool getConnection() const;

        bool IsAcceptor() const { return (!_connection); }

      private:
        int _port, _opt;
        std::uint64_t _reads = 0;
        int _fd = -1;
        bool _connection = false;
        struct sockaddr_in _address;
        bool _blocking = false;
};
typedef std::reference_wrapper<Socket> SocketRef;
}

#endif // SOCKET_SOCKET_H
