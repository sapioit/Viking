/*
Copyright (C) 2015 Voinea Constantin Vladimir

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/
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
