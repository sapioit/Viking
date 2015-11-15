#include <server/dispatcher.h>
#include <thread>
#include <sstream>
#include <http/engine.h>

Web::Dispatcher::RouteMap Web::Dispatcher::routes;

Web::Dispatcher::SchedulerResponse Web::Dispatcher::Dispatch(const Connection &connection)
{
	auto parser = Http::Engine(connection);
	// Http::Parser(connection);
	auto request = parser();
	if (request.IsPassable()) {
		if (!request.IsResource()) {
			auto handler = RouteUtility::GetHandler(request, routes);
			if (handler) {
				auto response = handler(request);
				return {ResponseSerializer::Serialize(response), response.should_close()};
			} else {
				/* No user-defined handler, return not found */
				return {ResponseSerializer::Serialize({request, Http::StatusCode::NotFound})};
			}
		} else {
			/* It's a resource */
			try {
				auto resource = CacheManager::GetResource(request.URI);
				// TODO decide if you should really close the connection
				return {ResponseSerializer::Serialize(request, resource)};
			} catch (Http::StatusCode code) {
				return {ResponseSerializer::Serialize({request, code})};
			}
		}
	} else {
		/* The request is not passable to the user and
		* it is not a resource either
		* TODO see what the standards says about this?
		*/
	}
	return {};
}
