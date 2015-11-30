#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <io/socket/socket.h>
#include <io/schedulers/sched_item.h>
#include <io/schedulers/io_scheduler.h>
#include <http/parser.h>
#include <http/engine.h>
#include <http/cachemanager.h>
#include <http/routeutility.h>
#include <http/resolution.h>
#include <map>
#include <memory>
#include <functional>

namespace Web {
class Dispatcher {
    static RouteMap routes;
    static std::vector<Http::Engine> pending_;
    typedef std::function<Http::Resolution(Http::Request)> Handler;

    static ScheduleItem TakeResource(const Http::Request &) noexcept;
    static ScheduleItem PassRequest(const Http::Request &, Handler) noexcept;
    static Http::Engine *GetPendingEngine(const IO::Socket *);
    static ScheduleItem ProcessEngine(const IO::Socket *, Http::Engine *, bool = false);

    static constexpr auto ready = 0;
    static constexpr auto not_ready = 1;
    static constexpr auto not_future = 2;

    public:
    template <typename T> static void AddRoute(T route) noexcept { routes.insert(route); }
    static ScheduleItem HandleConnection(const IO::Socket *) noexcept;
    static bool HandleBarrier(ScheduleItem &, std::unique_ptr<MemoryBuffer> &) noexcept;
};
}

#endif // DISPATCHER_H
