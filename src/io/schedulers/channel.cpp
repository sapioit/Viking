#include <io/schedulers/channel.h>

IO::Channel::Channel(std::unique_ptr<IO::Socket> socket) : socket(std::move(socket)) {}

IO::Channel::Channel(std::unique_ptr<IO::Socket> socket, std::uint8_t flags) : socket(std::move(socket)), flags(flags) {}

bool IO::Channel::operator=(const IO::Channel &other) const {
    return (*socket == *other.socket) && (flags == other.flags);
}
