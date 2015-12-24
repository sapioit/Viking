#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <io/schedulers/sched_item.h>
#include <http/engine.h>
#include <http/resolution.h>
#include <http/routeutility.h>

#include <map>
#include <memory>
#include <functional>

namespace Web {
class Dispatcher {
    RouteUtility::RouteMap routes;
    std::vector<Http::Engine> pending_;
    typedef std::function<Http::Resolution(Http::Request)> Handler;

    ScheduleItem TakeResource(const Http::Request &) noexcept;
    ScheduleItem PassRequest(const Http::Request &, Handler) noexcept;
    Http::Engine *GetPendingEngine(const IO::Socket *);
    ScheduleItem ProcessEngine(const IO::Socket *, Http::Engine *, bool = false);

    public:
    class Socket;
    void AddRoute(RouteUtility::Route) noexcept;
    ScheduleItem HandleConnection(const IO::Socket *) noexcept;
    std::unique_ptr<MemoryBuffer> HandleBarrier(AsyncBuffer<Http::Response> *) noexcept;
};
}

#endif // DISPATCHER_H
