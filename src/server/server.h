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
    Dispatcher dispatcher_;

    public:
    Server(int);
    ~Server() = default;
    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;
    Server(Server &&) = default;
    Server &operator=(Server &&) = default;
    void AddRoute(Http::Method method, const std::string &uri_regex,
                  std::function<Http::Resolution(Http::Request)> function);
    void SetSettings(const Settings &);
    void Run();
};
};

#endif // SERVER_H
