#ifndef RESOLUTION_H
#define RESOLUTION_H

#include <http/response.h>
#include <future>
namespace Http {
class Resolution {
    public:
    enum Type { Sync, Async };
    Http::Response response;
    std::future<Http::Response> future;
    Type type;
    Resolution(const Http::Response &);
    Resolution(std::future<Http::Response>);
};
}

#endif // RESOLUTION_H
