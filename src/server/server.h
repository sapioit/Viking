#ifndef SERVER_H
#define SERVER_H

#include <misc/settings.h>
#include <http/dispatcher/dispatcher.h>

#include <vector>
#include <memory>

namespace Web {
class Server {
    int port_;
    int max_pending_;
    Dispatcher *dispatcher_;

    public:
    Server(int);
    ~Server();
    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;
    Server(Server &&);
    Server &operator=(Server &&);
    template <class T> void AddRoute(T route) { dispatcher_->AddRoute(std::forward<T>(route)); }
    void SetSettings(const Settings &);
    void Run();
};
};

#endif // SERVER_H
