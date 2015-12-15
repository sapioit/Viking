#ifndef CONTEXT_H
#define CONTEXT_H

#include <memory>
#include <io/socket/socket.h>
#include <io/schedulers/sched_item.h>

namespace IO {
struct Channel {
    std::unique_ptr<Socket> socket;
    ScheduleItem queue;
    std::uint8_t flags = 0;
    static constexpr std::uint8_t Barrier = 1 << 1;
    static constexpr std::uint8_t Full = 1 << 2;
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
