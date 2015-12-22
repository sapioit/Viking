#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <io/schedulers/sched_item.h>
#include <http/engine.h>
#include <http/resolution.h>

#include <map>
#include <memory>
#include <functional>

namespace Web {
class Dispatcher {
    std::map<std::pair<Http::Method, std::string>, std::function<Http::Resolution(Http::Request)>> routes;
    std::vector<Http::Engine> pending_;
    typedef std::function<Http::Resolution(Http::Request)> Handler;

    ScheduleItem TakeResource(const Http::Request &) noexcept;
    ScheduleItem PassRequest(const Http::Request &, Handler) noexcept;
    Http::Engine *GetPendingEngine(const IO::Socket *);
    ScheduleItem ProcessEngine(const IO::Socket *, Http::Engine *, bool = false);

    public:
    class Socket;
    template <typename T> void AddRoute(T route) noexcept { routes.insert(route); }
    ScheduleItem HandleConnection(const IO::Socket *) noexcept;
    std::unique_ptr<MemoryBuffer> HandleBarrier(AsyncBuffer<Http::Response> *) noexcept;
};
}

#endif // DISPATCHER_H
