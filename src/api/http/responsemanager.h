#ifndef RESPONSEMANAGER_H
#define RESPONSEMANAGER_H

#include <http/response.h>
#include <http/request.h>
#include <io/socket/socket.h>

class ResponseManager {
      public:
        static void Respond(Http::Response, const IO::Socket &);
        static void Respond(const Http::Request &, const Resource &,
                            const IO::Socket &);
};

#endif // RESPONSEMANAGER_H
