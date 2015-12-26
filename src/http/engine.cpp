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
#ifndef ENGINE_H
#define ENGINE_H

#include <http/engine.h>
#include <http/components.h>
#include <http/util.h>
#include <misc/debug.h>
#include <misc/string_util.h>
#include <sstream>
#include <string>

Http::Context *GetMe(http_parser *parser) { return static_cast<Http::Context *>(parser->data); }

void Http::Context::AssignMethod(http_method method_numeric) {
    auto method = Http::MethodMap.find(http_method_str(method_numeric));
    if (method != Http::MethodMap.end())
        request_.method = method->second;
}

Http::Context::Context(const IO::Socket *socket) : socket_(socket), complete_(false) {
    settings_.on_message_begin = [](http_parser *) -> int { return 0; };
    settings_.on_message_complete = [](http_parser *) -> int {
        return 0;

    };
    settings_.on_headers_complete = [](http_parser *parser) -> int {
        auto me = GetMe(parser);
        me->request_.version.major = parser->http_major;
        me->request_.version.minor = parser->http_minor;
        me->AssignMethod(static_cast<http_method>(parser->method));
        return 0;
    };
    settings_.on_url = [](http_parser *parser, const char *at, size_t length) -> int {
        auto me = GetMe(parser);
        me->request_.url = {at, at + length};
        return 0;
    };
    settings_.on_header_field = [](http_parser *parser, const char *at, size_t length) -> int {
        auto me = GetMe(parser);
        me->header_field = std::string{at, at + length};

        return 0;
    };
    settings_.on_header_value = [](http_parser *parser, const char *at, size_t length) -> int {
        std::string value(at, at + length);
        auto me = GetMe(parser);
        me->request_.header.fields.insert(std::make_pair(me->header_field, value));

        return 0;
    };
    settings_.on_body = [](http_parser *parser, const char *at, size_t length) -> int {
        auto me = GetMe(parser);
        me->request_.body += {at, at + length};
        /* TODO check if body size is OK, and if yes, then copy it to the request. If not,
         * we should provide the user with a way of getting the body on demand (either async,
         * by returning a future -> implies starting an I/O scheduler async, or by
         * temporarily making the socket blocking and reading all the data
         */

        return 0;

    };
}

const IO::Socket *Http::Context::GetSocket() const { return socket_; }

const Http::Request &Http::Context::GetRequest() const noexcept { return request_; }

Http::Context &Http::Context::operator()() {
    try {
        buffer += socket_->ReadSome<std::string>();
        parser_.data = reinterpret_cast<void *>(this);
        http_parser_init(&parser_, HTTP_REQUEST);
        http_parser_execute(&parser_, &settings_, &buffer.front(), buffer.size());
        complete_ = Http::Util::IsComplete(request_);
        return *this;
        /* TODO in the future, take the state into account, because we
             * might not be getting the full header
             */
    } catch (...) {
        throw;
    }
}

bool Http::Context::Complete() const noexcept { return complete_; }

#endif
