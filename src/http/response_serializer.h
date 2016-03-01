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
#ifndef RESPONSEMANAGER_H
#define RESPONSEMANAGER_H

#include <http/response.h>
#include <http/request.h>

class response_serializer {
    public:
    response_serializer() = default;
    virtual ~response_serializer() = default;

    std::vector<char> make_header(const http::response &response) noexcept;
    std::vector<char> make_body(const http::response &response) noexcept;
    std::vector<char> make_ending(const http::response &response) noexcept;
    std::vector<char> operator()(const http::response &response) noexcept;
};

#endif // RESPONSEMANAGER_H
