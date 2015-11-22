#ifndef ENGINE_H
#define ENGINE_H

#include <http/engine.h>
#include <http/components.h>
#include <misc/debug.h>
#include <sstream>

Http::Engine *Http::Engine::GetMe(http_parser *parser) { return reinterpret_cast<Engine *>(parser->data); }

void Http::Engine::AssignMethod(http_method method_numeric) {
    auto *method_str = http_method_str(method_numeric);
    auto method = Http::MethodMap.find(method_str);
    if (method != Http::MethodMap.end())
        request_.method = method->second;
}

Http::Engine::Engine(const IO::Socket &socket) : socket_(socket) {
    settings_.on_message_begin = [](http_parser *) -> int { return 0; };
    settings_.on_message_complete = [](http_parser *) -> int {
        return 0;

    };
    settings_.on_headers_complete = [](http_parser *parser) -> int {
        auto *me = GetMe(parser);
        me->request_.version.major = parser->http_major;
        me->request_.version.minor = parser->http_minor;
        me->AssignMethod(static_cast<http_method>(parser->method));
        return 0;
    };
    settings_.on_url = [](http_parser *parser, const char *at, size_t length) -> int {
        auto *me = GetMe(parser);
        me->request_.url = {at, at + length};
        return 0;

    };
    settings_.on_header_field = [](http_parser *parser, const char *at, size_t length) -> int {
        auto *me = GetMe(parser);
        me->header_field = std::string{at, at + length};
        debug(me->header_field);

        return 0;
    };
    settings_.on_header_value = [](http_parser *parser, const char *at, size_t length) -> int {
        std::string value(at, at + length);
        auto *me = GetMe(parser);
        me->request_.header.fields.insert(std::make_pair(me->header_field, value));

        debug(value);
        return 0;
    };
    settings_.on_body = [](http_parser *, const char *at, size_t length) -> int {
        std::string body(at, at + length);
        debug(body);

        return 0;

    };
    parser_.data = reinterpret_cast<void *>(this);
    http_parser_init(&parser_, HTTP_REQUEST);
}

Http::Request Http::Engine::operator()() {
    try {
        auto data = socket_.ReadSome<std::string>();
        http_parser_execute(&parser_, &settings_, &data.front(), data.size());
        return request_;
        /* TODO in the future, take the state into account, because we
             * might not be getting the full header
             */
    } catch (...) {
        throw;
    }
    return {};
}


std::string Http::Engine::StripRoute(const std::string &URI) {
    auto firstSlash = URI.find_first_of('/');
    return {URI.begin() + firstSlash, URI.end()};
}

std::vector<Http::ContentType> Http::Engine::GetAcceptedEncodings(const std::string &) {
    return std::vector<Http::ContentType>();
}

Http::ContentType Http::Engine::GetMimeType(const std::string &) {
    // TODO parse line and get it
    return Http::ContentType::ApplicationJson;
}

#endif
