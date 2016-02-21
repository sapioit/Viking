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

class route_util {
    public:
    typedef std::function<bool(std::string)> route_validator;
    typedef std::pair<http::method, route_validator> method_route_validator;
    typedef std::function<http::resolution(http::request)> http_handler;
    typedef std::pair<method_route_validator, http_handler> route;

    typedef std::vector<route> route_map;

    static http_handler get_user_handler(const http::request &request, const route_map &routes);

    static std::string strip_route(const std::string &);
};

#endif // ROUTESMANAGER_H
