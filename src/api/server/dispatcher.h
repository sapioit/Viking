#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <io/socket/socket.h>
#include <io/schedulers/io_scheduler.h>
#include <http/routeutility.h>
#include <http/parser.h>
#include <http/routeutility.h>
#include <http/parser.h>
#include <http/components.h>
#include <http/cachemanager.h>
#include <http/response_serializer.h>
#include <misc/log.h>
#include <misc/storage.h>
#include <map>
#include <memory>
#include <functional>

namespace Web
{
class Dispatcher
{
	using DataType = IO::Scheduler::DataType;
	using Connection = IO::Socket;
	typedef std::map<std::pair<Http::Components::Method, std::string>, std::function<Http::Response(Http::Request)>>
	    RouteMap;
	static RouteMap routes;
	typedef IO::Scheduler::CallbackResponse SchedulerResponse;

	public:
	template <typename T> static void AddRoute(T route) { routes.insert(route); }
	static IO::Scheduler::CallbackResponse Dispatch(const Connection &Connection);
};
}

#endif // DISPATCHER_H
