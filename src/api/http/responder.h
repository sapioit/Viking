#ifndef RESPONSEMANAGER_H
#define RESPONSEMANAGER_H

#include <http/response.h>
#include <http/request.h>
#include <http/parser.h>
#include <io/socket/socket.h>
#include <io/schedulers/out.h>

template <typename Stream> class Responder {
        Stream stream;

      public:
        Responder(Stream stream) : stream(stream){};
        void Respond(Http::Response response, const IO::Socket &socket) {
                try {
                        auto raw_response = response.str();
                        auto raw_resp_vec = std::vector<char>(
                            raw_response.begin(), raw_response.end());
                        /*IO::OutputScheduler::get().ScheduleWrite(socket,
                                                                 raw_response);*/
                        stream(std::move(socket.Duplicate()), raw_resp_vec);

                } catch (std::exception &ex) {
                        Respond({response.getRequest(),
                                 Http::StatusCode::InternalServerError},
                                socket);
                }
        }
        void Respond(const Http::Request &request, const Resource &res,
                     const IO::Socket &socket) {
                Http::Response response{request, res};
                response.setContent_type(
                    Http::Parser::GetMimeTypeByExtension(request.URI));
                Respond(response, socket);
        }
};

#endif // RESPONSEMANAGER_H
