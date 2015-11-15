#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <io/socket/socket.h>
#include <io/schedulers/out.h>
#include <http/routeutility.h>
#include <http/parser.h>
#include <http/routeutility.h>
#include <http/parser.h>
#include <http/components.h>
#include <http/cachemanager.h>
#include <http/responder.h>
#include <io/schedulers/out.h>
#include <misc/log.h>
#include <misc/storage.h>
#include <map>
#include <memory>
#include <functional>

namespace Web
{
class Dispatcher
{
	using DataType = std::vector<char>;
	using Connection = IO::Socket;
	std::map<std::pair<Http::Components::Method, std::string>, std::function<Http::Response(Http::Request)>> routes;
    IO::Scheduler::Out<DataType> output_sched;
	std::thread output_thread;

      public:
	Dispatcher();

	template <typename T> void AddRoute(T route) { routes.insert(route); }
	bool Dispatch(const Connection &connection);
};
}

#endif // DISPATCHER_H
