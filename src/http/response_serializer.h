#ifndef RESPONSEMANAGER_H
#define RESPONSEMANAGER_H

#include <http/response.h>
#include <http/request.h>

class ResponseSerializer {
    public:
    ResponseSerializer() = default;
    virtual ~ResponseSerializer() = default;

    std::vector<char> MakeHeader(const Http::Response &response) noexcept;
    std::vector<char> MakeBody(const Http::Response &response) noexcept;
    std::vector<char> MakeEnding(const Http::Response &response) noexcept;
    std::vector<char> operator()(const Http::Response &response) noexcept;
};

#endif // RESPONSEMANAGER_H
