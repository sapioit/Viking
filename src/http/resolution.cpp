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

using namespace http;
resolution::resolution(http::response &&r) : m_response(std::move(r)), m_type(type::sync) {}

resolution::resolution(std::future<response> &&future) : future(std::move(future)), m_type(type::async) {}

resolution::type resolution::get_type() const noexcept { return m_type; }

std::future<response> &resolution::get_future() noexcept { return future; }

response &resolution::get_response() noexcept { return m_response; }
