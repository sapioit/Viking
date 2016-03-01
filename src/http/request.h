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

#ifndef SOCKET_REQUEST_H
#define SOCKET_REQUEST_H

#include <http/header.h>
#include <http/version.h>
#include <string>
#include <vector>

namespace http {

class request {
    public:
    http::method method;
    version m_version;
    header m_header;
    std::string url, body;

    request() = default;
    virtual ~request() = default;

    /* For convenience */
    std::vector<std::string> split_url() const;
};
}

#endif // SOCKET_REQUEST_H
