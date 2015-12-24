#ifndef ENGINE_H
#define ENGINE_H

#include <http/engine.h>
#include <http/components.h>
#include <misc/debug.h>
#include <misc/string_util.h>
#include <sstream>

Http::Engine *GetMe(http_parser *parser) { return static_cast<Http::Engine *>(parser->data); }

void Http::Engine::AssignMethod(http_method method_numeric) {
    auto method = Http::MethodMap.find(http_method_str(method_numeric));
    if (method != Http::MethodMap.end())
        request_.method = method->second;
}

Http::Engine::Engine(const IO::Socket *socket) : socket_(socket) {
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
        me->request_.url = StringUtil::DecodeURL({at, at + length});
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

const IO::Socket *Http::Engine::GetSocket() const { return socket_; }

Http::Request Http::Engine::operator()() {
    try {
        buffer += socket_->ReadSome<std::string>();
        parser_.data = reinterpret_cast<void *>(this);
        http_parser_init(&parser_, HTTP_REQUEST);
        http_parser_execute(&parser_, &settings_, &buffer.front(), buffer.size());
        return request_;
        /* TODO in the future, take the state into account, because we
             * might not be getting the full header
             */
    } catch (...) {
        throw;
    }
    return {};
}

#endif
