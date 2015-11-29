#ifndef CONTEXT_H
#define CONTEXT_H

#include <memory>
#include <io/socket/socket.h>

namespace IO {
struct Channel {
    std::unique_ptr<Socket> socket;
    std::uint8_t flags;
    static constexpr std::uint8_t Barrier = 1 << 1;
    Channel() = default;
    Channel(std::unique_ptr<Socket> socket);
    Channel(std::unique_ptr<Socket> socket, std::uint8_t flags);
    Channel(const Channel &) = delete;
    Channel &operator=(const Channel &) = delete;
    Channel(Channel &&other) = default;
    Channel &operator=(Channel &other) = default;
    ~Channel() = default;
    bool operator=(const Channel &other) const;
};
}

#endif // CONTEXT_H
