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
#ifndef SETTINGS_H
#define SETTINGS_H
#include <string>
#include <http/request.h>
#include <http/resolution.h>

struct Settings {
    Settings();
    ~Settings() = default;

    std::string root_path;
    std::uint32_t max_connections;
    std::uint32_t default_max_age = 300;
    bool allow_directory_listing;
    std::function<http::resolution(http::request)> folder_cb;
};

#endif // SETTINGS_H
