#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <io/socket/socket.h>
#include <io/schedulers/io_scheduler.h>
#include <http/parser.h>
#include <http/cachemanager.h>
#include <http/routeutility.h>
#include <map>
#include <memory>
#include <functional>

namespace Web {
class Dispatcher {
    using DataType = IO::Scheduler::DataType;
    using Connection = IO::Socket;
    static RouteMap routes;
    typedef IO::Scheduler::Resolution SchedulerResponse;
    typedef std::function<Http::Response(Http::Request)> Handler;

    static SchedulerResponse TakeResource(const Http::Request &) noexcept;
    static SchedulerResponse PassRequest(const Http::Request &, Handler) noexcept;

    static constexpr auto ready = 0;
    static constexpr auto not_ready = 1;
    static constexpr auto not_future = 2;

    public:
    template <typename T> static void AddRoute(T route) noexcept { routes.insert(route); }
    static SchedulerResponse HandleConnection(const Connection *Connection) noexcept;
    static bool HandleBarrier(ScheduleItem&, std::unique_ptr<MemoryBuffer> &) noexcept;
};
}

#endif // DISPATCHER_H
