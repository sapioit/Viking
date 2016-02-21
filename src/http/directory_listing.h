#ifndef DIRECTORY_LISTING_H
#define DIRECTORY_LISTING_H

#include <http/resolution.h>
#include <http/request.h>

namespace http {
http::response list_directory(const http::request &req);
}

#endif // DIRECTORY_LISTING_H
