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
#include <iterator>
#include <misc/common.h>
#include <misc/date.h>
#include <misc/string_util.h>
#include <sstream>
#include <string.h>
#include <unistd.h>

static constexpr auto crlf = "\r\n";
static constexpr auto crlfcrlf = "\r\n\r\n";

std::vector<char> response_serializer::make_header(const http::response &r) noexcept {
    std::string response;
    response.reserve(256);
    response.append("HTTP/")
        .append(std::to_string(r.version.major))
        .append(".")
        .append(std::to_string(r.version.minor));
    response.append(" ").append(std::to_string(r.get_code())).append(" ");
    response.append(http::status_codes.at(r.get_code())).append(crlf);
    for (const auto &pair : r.fields)
        response.append(pair.first).append(": ").append(pair.second).append(crlf);
    response.append(crlf);
    return {response.begin(), response.end()};
}

std::vector<char> response_serializer::make_body(const http::response &response) noexcept {
    return response.body();
    switch (response.get_type()) {
    case http::response::type::resource:
        return response.body();
    case http::response::type::text:
        return {response.get_text().begin(), response.get_text().end()};
    default:
        return {};
    }
}

std::vector<char> response_serializer::make_ending(const http::response &) noexcept {
    return std::vector<char>(crlfcrlf, crlfcrlf + strlen(crlfcrlf));
}

std::vector<char> response_serializer::operator()(const http::response &response) noexcept {
    auto header = make_header(response);
    auto body = make_body(response);
    auto ending = make_ending(response);

    std::vector<char> buffer;
    buffer.reserve(getpagesize());
    buffer.insert(buffer.end(), std::make_move_iterator(header.begin()), std::make_move_iterator(header.end()));
    buffer.insert(buffer.end(), std::make_move_iterator(body.begin()), std::make_move_iterator(body.end()));
    buffer.insert(buffer.end(), std::make_move_iterator(ending.begin()), std::make_move_iterator(ending.end()));
    return buffer;
}
