#ifndef DIRECTORY_LISTING_H
#define DIRECTORY_LISTING_H

#include <http/resolution.h>
#include <http/request.h>

namespace Http {
Http::Response list_directory(const Http::Request &req);
}

#endif // DIRECTORY_LISTING_H
