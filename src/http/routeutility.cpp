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
#include "routeutility.h"
#include "parser.h"
#include <http/engine.h>
#include <misc/debug.h>
#include <algorithm>
#include <regex>

std::function<http::resolution(http::request)> route_util::get_user_handler(const http::request &request,
                                                                            const route_map &routes) {
    auto strippedRoute = strip_route(request.url);
    auto result = std::find_if(routes.begin(), routes.end(), [&](const auto &route) -> bool {
        return request.method == route.first.first && route.first.second(strippedRoute);
    });

    if (result != routes.end()) {
        return result->second;
    }

    return nullptr;
}

std::string route_util::strip_route(const std::string &URI) {
    auto firstSlash = URI.find_first_of('/');
    return {URI.begin() + firstSlash, URI.end()};
}
