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
#ifndef RESOLUTION_H
#define RESOLUTION_H

#include <http/response.h>
#include <future>
namespace http {
class resolution {
    http::response m_response;
    std::future<http::response> future;

    public:
    enum type { Sync, Async };

    private:
    type m_type;

    public:
    resolution(http::response &&);
    resolution(std::future<http::response> &&);

    type GetType() const noexcept;
    std::future<http::response> &GetFuture() noexcept;
    http::response &GetResponse() noexcept;
};
}

#endif // RESOLUTION_H
