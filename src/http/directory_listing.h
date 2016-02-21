#ifndef DIRECTORY_LISTING_H
#define DIRECTORY_LISTING_H

#include <http/resolution.h>
#include <http/request.h>

namespace http {
http::Response list_directory(const http::Request &req);
}

#endif // DIRECTORY_LISTING_H
