#include <server/dispatcher.h>
#include <thread>
#include <sstream>

Web::Dispatcher::Dispatcher()
    : output_sched(),
      output_thread(&IO::Scheduler::Out<Connection, DataType>::Run,
                    std::ref(output_sched)) {
        output_thread.detach();
}

bool Web::Dispatcher::Dispatch(const Connection &connection) {
        try {
                auto parser = Http::Parser(connection);
                auto request = parser();
                using namespace std::placeholders;
                Responder<std::function<void(Connection, const DataType &)>>
                    responder(std::bind(
                        &IO::Scheduler::Out<Connection, DataType>::Add,
                        &output_sched, _1, _2));
                if (request.IsPassable()) {
                        if (!request.IsResource()) {
                                auto handler =
                                    RouteUtility::GetHandler(request, routes);
                                if (handler) {
                                        auto response = handler(request);
                                        responder.Respond(response, connection);
                                        return response.should_close();
                                } else {
                                        // no user-defined handler,
                                        // return not
                                        // found
                                        responder.Respond({request, 404},
                                                          connection);
                                        return true;
                                }
                        } else {
                                // it's a resource
                                try {
                                        auto resource =
                                            CacheManager::GetResource(
                                                request.URI);
                                        responder.Respond(request, resource,
                                                          connection);
                                        return true;
                                } catch (Http::StatusCode code) {
                                        responder.Respond({request, code},
                                                          connection);
                                        return false;
                                }
                        }
                } else {
                        // The request is not passable to the user and
                        // it is not
                        // a resource either
                        // TODO see what the standards says about this?
                }
        } catch (std::runtime_error &ex) {
                Log::e(ex.what());
        } catch (IO::Socket::ConnectionClosedByPeer) {
                return true;
        }
        return true;
}
