//
// Created by vladimir on 09.08.2015.
//

#ifndef SOCKET_HTTPUTILITY_H
#define SOCKET_HTTPUTILITY_H

#include <io/socket/socket.h>
#include <http/request.h>
#include <http/parser.h>
#include <string>

namespace Http {
class Engine {
    const IO::Socket *socket_;
    http_parser_settings settings_;
    http_parser parser_;
    Request request_;
    std::string buffer;
    std::string header_field;

    void AssignMethod(http_method method_numeric);

    public:
    Engine(const IO::Socket *socket);
    const IO::Socket *GetSocket() const;
    Request operator()();
};
};

#endif // SOCKET_HTTPUTILITY_H
