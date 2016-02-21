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
#ifndef COMPONENTS
#define COMPONENTS

#include <map>

namespace http {
enum class method {
    Delete,
    Get,
    Head,
    Post,
    Put,
    Connect,
    Options,
    Trace,
    Copy,
    Lock,
    Mkcol,
    Move,
    Propfind,
    Proppatch,
    Unlock,
    Report,
    Mkactivity,
    Checkout,
    Merge,
    M_Search,
    Notify,
    Subscribe,
    Unsubscribe
};

const std::map<const std::string, const method> MethodMap{std::make_pair("GET", method::Get),
                                                          std::make_pair("POST", method::Post),
                                                          std::make_pair("PUT", method::Put),
                                                          std::make_pair("DELETE", method::Delete),
                                                          std::make_pair("Connect", method::Connect),
                                                          std::make_pair("HEAD", method::Head),
                                                          std::make_pair("OPTIONS", method::Options),
                                                          std::make_pair("TRACE", method::Trace),
                                                          std::make_pair("COPY", method::Copy),
                                                          std::make_pair("LOCK", method::Lock),
                                                          std::make_pair("MKCOL", method::Mkcol),
                                                          std::make_pair("MOVE", method::Move),
                                                          std::make_pair("PROPFIND", method::Propfind),
                                                          std::make_pair("PROPPATCH", method::Proppatch),
                                                          std::make_pair("UNLOCK", method::Unlock),
                                                          std::make_pair("REPORT", method::Report),
                                                          std::make_pair("MKACTIVITY", method::Mkactivity),
                                                          std::make_pair("CHECKOUT", method::Checkout),
                                                          std::make_pair("MERGE", method::Merge),
                                                          std::make_pair("M-SEARCH", method::M_Search),
                                                          std::make_pair("NOTIFY", method::Notify),
                                                          std::make_pair("SUBSCRIBE", method::Subscribe),
                                                          std::make_pair("UNSUBSCRIBE", method::Unsubscribe)};

enum status_code {
    OK = 200,
    Found = 302,
    BadRequest = 400,
    NotFound = 404,
    UnsupportedMediaType = 415,
    InternalServerError = 500
};

const std::map<const status_code, const std::string> StatusCodes{
    std::make_pair(status_code::OK, "OK"),
    std::make_pair(status_code::BadRequest, "Bad Request"),
    std::make_pair(status_code::Found, "Found"),
    std::make_pair(status_code::NotFound, "Not Found"),
    std::make_pair(status_code::UnsupportedMediaType, "Unsupported Media Type"),
    std::make_pair(status_code::InternalServerError, "Internal Server Error")};
}
#endif // COMPONENTS
