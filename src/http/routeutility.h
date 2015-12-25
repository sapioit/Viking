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
#ifndef ROUTESMANAGER_H
#define ROUTESMANAGER_H

#include <http/request.h>
#include <http/response.h>
#include <http/resolution.h>
#include <map>
#include <functional>

class RouteUtility {
    public:
    typedef std::function<bool(std::string)> RouteValidator;
    typedef std::pair<Http::Method, RouteValidator> MethodRouteValidatorPair;
    typedef std::function<Http::Resolution(Http::Request)> HttpHandler;
    typedef std::pair<MethodRouteValidatorPair, HttpHandler> Route;

    typedef std::vector<Route> RouteMap;

    static HttpHandler GetHandler(const Http::Request &request, const RouteMap &routes);

    static std::string StripRoute(const std::string &URI);
};

#endif // ROUTESMANAGER_H
