//
// Created by vladimir on 08.08.2015.
//

#ifndef SOCKET_REQUEST_H
#define SOCKET_REQUEST_H

#include <http/components.h>
#include <http/header.h>
#include <http/version.h>
#include <string>

namespace Http {

class Request {
    public:
    Http::Method method;
    Version version;
    Header header;
    std::string url, body;

    Request() = default;
    virtual ~Request() = default;

    /* For convenience */
    std::vector<std::string> SplitURL() const;
};
}

#endif // SOCKET_REQUEST_H
