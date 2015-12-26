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
#include <http/response_serializer.h>
#include <http/components.h>
#include <misc/string_util.h>
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
        return response.GetResource().Content();
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
