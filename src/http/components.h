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
enum class Method {
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

const std::map<const std::string, const Method> MethodMap{std::make_pair("GET", Method::Get),
                                                          std::make_pair("POST", Method::Post),
                                                          std::make_pair("PUT", Method::Put),
                                                          std::make_pair("DELETE", Method::Delete),
                                                          std::make_pair("Connect", Method::Connect),
                                                          std::make_pair("HEAD", Method::Head),
                                                          std::make_pair("OPTIONS", Method::Options),
                                                          std::make_pair("TRACE", Method::Trace),
                                                          std::make_pair("COPY", Method::Copy),
                                                          std::make_pair("LOCK", Method::Lock),
                                                          std::make_pair("MKCOL", Method::Mkcol),
                                                          std::make_pair("MOVE", Method::Move),
                                                          std::make_pair("PROPFIND", Method::Propfind),
                                                          std::make_pair("PROPPATCH", Method::Proppatch),
                                                          std::make_pair("UNLOCK", Method::Unlock),
                                                          std::make_pair("REPORT", Method::Report),
                                                          std::make_pair("MKACTIVITY", Method::Mkactivity),
                                                          std::make_pair("CHECKOUT", Method::Checkout),
                                                          std::make_pair("MERGE", Method::Merge),
                                                          std::make_pair("M-SEARCH", Method::M_Search),
                                                          std::make_pair("NOTIFY", Method::Notify),
                                                          std::make_pair("SUBSCRIBE", Method::Subscribe),
                                                          std::make_pair("UNSUBSCRIBE", Method::Unsubscribe)};

enum StatusCode {
    OK = 200,
    Found = 302,
    BadRequest = 400,
    NotFound = 404,
    UnsupportedMediaType = 415,
    InternalServerError = 500
};

const std::map<const StatusCode, const std::string> StatusCodes{
    std::make_pair(StatusCode::OK, "OK"),
    std::make_pair(StatusCode::BadRequest, "Bad Request"),
    std::make_pair(StatusCode::Found, "Found"),
    std::make_pair(StatusCode::NotFound, "Not Found"),
    std::make_pair(StatusCode::UnsupportedMediaType, "Unsupported Media Type"),
    std::make_pair(StatusCode::InternalServerError, "Internal Server Error")};
}
#endif // COMPONENTS
