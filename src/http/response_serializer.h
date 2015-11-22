#ifndef RESPONSEMANAGER_H
#define RESPONSEMANAGER_H

#include <http/response.h>
#include <http/request.h>
#include <http/engine.h>

class ResponseSerializer {
    public:
    typedef std::vector<char> DataType;
    static DataType Serialize(const Http::Response &response) {
        try {
            auto raw_response = response.str();
            auto raw_resp_vec = DataType(raw_response.begin(), raw_response.end());

            return raw_resp_vec;
        } catch (...) {
            return Serialize({response.getRequest(), Http::StatusCode::InternalServerError});
        }
    }
    static DataType Serialize(const Http::Request &request, const Resource &res) {
        Http::Response response{request, res};
        response.SetContentType(Http::Engine::GetMimeTypeByExtension(request.url));
        return Serialize(response);
    }
};

#endif // RESPONSEMANAGER_H
