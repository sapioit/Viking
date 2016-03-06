#ifndef STATUS_CODES_H
#define STATUS_CODES_H
#include <misc/common.h>

namespace http {
enum status_code {
    OK = 200,
    Found = 302,
    BadRequest = 400,
    NotFound = 404,
    UnsupportedMediaType = 415,
    InternalServerError = 500
};

const std::unordered_map<status_code, std::string> status_codes{
    {status_code::OK, "OK"},
    {status_code::BadRequest, "Bad Request"},
    {status_code::Found, "Found"},
    {status_code::NotFound, "Not Found"},
    {status_code::UnsupportedMediaType, "Unsupported Media Type"},
    {status_code::InternalServerError, "Internal Server Error"}};
}
#endif // STATUS_CODES_H
