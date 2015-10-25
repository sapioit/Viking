#include <http/responsemanager.h>
#include <http/parser.h>
#include <io/outputscheduler.h>
#include <misc/log.h>
#include <misc/storage.h>

#include <sstream>

using namespace Http::Components;

void ResponseManager::Respond(Http::Response response, IO::Socket &socket) {
  try {
    auto raw_response = response.str();
    IO::OutputScheduler::get().ScheduleWrite(socket, std::move(raw_response));
  } catch (std::exception &ex) {
    Log::e(ex.what());
    ResponseManager::Respond(
        {response.getRequest(), StatusCode::InternalServerError}, socket);
  }
}

void ResponseManager::Respond(const Http::Request &request, const Resource &res,
                              IO::Socket &socket) {
  Http::Response response{request, res};
  response.setContent_type(Http::Parser::GetMimeTypeByExtension(request.URI));
  ResponseManager::Respond(response, socket);
}