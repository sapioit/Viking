#include <server/dispatcher.h>
#include <http/parser.h>
#include <http/routeutility.h>
#include <http/parser.h>
#include <http/components.h>
#include <http/cachemanager.h>
#include <http/responsemanager.h>
#include <misc/log.h>
#include <misc/storage.h>

#include <thread>
#include <sstream>

std::map<std::pair<Http::Components::Method, std::string>,
         std::function<Http::Response(Http::Request)>> Web::Dispatcher::routes;

using namespace Web;
using namespace Http::Components;

constexpr auto close_after_resource = true;

bool Dispatcher::Dispatch(IO::Socket &connection) {
  try {
    auto parser = Http::Parser(connection);
    auto request = std::move(parser());
    if (request.IsPassable()) {
      if (!request.IsResource()) {
        auto handler = RouteUtility::GetHandler(request, routes);
        if (handler) {
          auto response = handler(request);
          ResponseManager::Respond(response, connection);
          return response.should_close();
        } else {
          // no user-defined handler, return not found
          ResponseManager::Respond({request, 404}, connection);
          return true;
        }
      } else {
        // it's a resource
        try {
          auto resource = std::move(CacheManager::GetResource(request.URI));
          ResponseManager::Respond(request, resource, connection);
          return close_after_resource;
        } catch (StatusCode code) {
          ResponseManager::Respond({request, code}, connection);
          return false;
        }
      }
    } else {
      // The request is not passable to the user and it is not a resource either
      // TODO see what the standards says about this?
    }
  } catch (std::runtime_error &ex) {
    Log::e(ex.what());
  } catch (IO::Socket::connection_closed_by_peer) {
    return true;
  }
  return true;
}

/*void Dispatcher::PassToUser(
    Http::Request request,
    std::function<Http::Response(Http::Request)> user_handler,
    IO::Socket &socket) {

  auto response = user_handler(request);

  ResponseManager::Respond(std::move(response), socket);
}
*/