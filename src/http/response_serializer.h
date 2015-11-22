#ifndef RESPONSEMANAGER_H
#define RESPONSEMANAGER_H

#include <http/response.h>
#include <http/request.h>
#include <http/engine.h>

class ResponseSerializer {
    public:
    ResponseSerializer() = default;
    virtual ~ResponseSerializer() = default;
    typedef std::vector<char> DataType;

    DataType operator()(const Http::Response &response) {
        try {
            auto raw_response = response.str();
            auto raw_resp_vec = DataType(raw_response.begin(), raw_response.end());

            return raw_resp_vec;
        } catch (...) {
            return (*this)(Http::StatusCode::InternalServerError);
        }
    }
};

#endif // RESPONSEMANAGER_H
