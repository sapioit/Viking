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
#include <algorithm>
#include <regex>

std::function<Http::Resolution(Http::Request)> RouteUtility::GetHandler(const Http::Request &request,
                                                                        const RouteMap &routes) {
    auto strippedRoute = StripRoute(request.url);
    auto result = std::find_if(routes.begin(), routes.end(), [&](const auto &route) -> bool {
        return request.method == route.first.first && route.first.second(strippedRoute);
    });

    if (result != routes.end()) {
        return result->second;
    }

    return nullptr;
}

std::string RouteUtility::StripRoute(const std::string &URI) {
    auto firstSlash = URI.find_first_of('/');
    return {URI.begin() + firstSlash, URI.end()};
}
