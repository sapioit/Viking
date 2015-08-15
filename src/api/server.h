#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <memory>
#include <experimental/optional>
#include "Socket.h"

namespace Web{
class Server
{
public:
    Server(int);
    void run();

    int maxPending() const;
    void setMaxPending(int maxPending);

private:
    int _port = -1;
    int _maxPending = 100;
    std::shared_ptr<IO::Socket> _masterSocket;
};
};

#endif // SERVER_H
