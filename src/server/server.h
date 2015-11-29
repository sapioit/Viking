#ifndef SERVER_H
#define SERVER_H

#include <http/dispatcher/dispatcher.h>
#include <misc/settings.h>

#include <vector>
#include <memory>

namespace Web {
class Server {
    public:
    Server(int);
    template <class T> void AddRoute(T route) { Dispatcher::AddRoute(std::forward<T>(route)); }
    void Run();
    void SetSettings(const Settings &);

    int GetMaxPending() const;
    void SetMaxPending(int);

    private:
    int _port = -1;
    int _maxPending;
};
};

#endif // SERVER_H
