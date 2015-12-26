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
#include <http/resolution.h>

using namespace Http;
Resolution::Resolution(Http::Response &&r) : response(std::move(r)), type(Type::Sync) {}

Resolution::Resolution(std::future<Response> &&future)
    : future(std::forward<std::future<Http::Response>>(future)), type(Type::Async) {}

Resolution::Type Resolution::GetType() const noexcept { return type; }

std::future<Response> &Resolution::GetFuture() noexcept { return future; }

Response &Resolution::GetResponse() noexcept { return response; }
