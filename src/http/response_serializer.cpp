#include <http/response_serializer.h>
#include <http/components.h>
#include <misc/string_util.h>
#include <misc/dfa.h>
#include <misc/date.h>
#include <http/engine.h>
#include <string.h>
#include <sstream>

static constexpr auto crlf = "\r\n";
static constexpr auto crlfcrlf = "\r\n\r\n";

std::vector<char> ResponseSerializer::MakeHeader(const Http::Response &response) noexcept {
    std::ostringstream stream;
    stream << "HTTP/" << response.GetVersion().major << "." << response.GetVersion().minor;
    stream << " " << response.GetCode() << " ";
    stream << Http::StatusCodes.at(response.GetCode()) << crlf;
    for (const auto &pair : response.GetFields()) {
        stream << pair.first << ": " << pair.second << crlf;
    }
    stream << crlf;
    auto string = stream.str();
    return {string.begin(), string.end()};
}

std::vector<char> ResponseSerializer::MakeBody(const Http::Response &response) noexcept {
    switch (response.GetType()) {
    case Http::Response::Type::Resource:
        return response.GetResource().content();
    case Http::Response::Type::Text:
        return {response.GetText().begin(), response.GetText().end()};
    default:
        return {};
    }
}

std::vector<char> ResponseSerializer::MakeEnding(const Http::Response &) noexcept {
    return std::vector<char>(crlfcrlf, crlfcrlf + strlen(crlfcrlf));
}

std::vector<char> ResponseSerializer::operator()(const Http::Response &response) noexcept {
    auto header = MakeHeader(response);
    auto body = MakeBody(response);
    auto ending = MakeEnding(response);

    std::vector<char> buffer;
    buffer.reserve(header.size() + body.size() + ending.size());
    buffer.insert(buffer.end(), header.begin(), header.end());
    buffer.insert(buffer.end(), body.begin(), body.end());
    buffer.insert(buffer.end(), ending.begin(), ending.end());
    return buffer;
}
