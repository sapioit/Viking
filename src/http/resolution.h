#ifndef RESOLUTION_H
#define RESOLUTION_H

#include <http/response.h>
#include <future>
namespace Http {
class Resolution {
    Http::Response response;
    std::future<Http::Response> future;

    public:
    enum Type { Sync, Async };

    private:
    Type type;

    public:
    Resolution(const Http::Response &);
    Resolution(std::future<Http::Response> &&);

    Type GetType() const noexcept;
    std::future<Http::Response> &GetFuture() noexcept;
    Http::Response &GetResponse() noexcept;
};
}

#endif // RESOLUTION_H
