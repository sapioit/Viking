#ifndef CONTEXT_H
#define CONTEXT_H

#include <memory>
#include <io/socket/socket.h>

namespace IO {
using Flags = std::uint8_t;
struct Channel {
    std::unique_ptr<Socket> socket;
    Flags flags;
    Channel() = default;
    Channel(std::unique_ptr<Socket> socket) : socket(std::move(socket)) {}
    Channel(std::unique_ptr<Socket> socket, Flags flags) : socket(std::move(socket)), flags(flags) {}
    Channel(const Channel &) = delete;
    Channel &operator=(const Channel &) = delete;
    Channel(Channel &&other) = default;
    Channel &operator=(Channel &other) = default;
    ~Channel() = default;
    bool operator=(const Channel &other) const { return (*socket == *other.socket) && (flags == other.flags); }
};
}

#endif // CONTEXT_H
