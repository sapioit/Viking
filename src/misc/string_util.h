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
#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <algorithm>
#include <misc/date.h>
#include <string>
#include <vector>

std::vector<std::string> split(const std::string &, char) noexcept;
std::string url_decode(const std::string &);

template <typename T> T uppercase(const T &item) {
    T ret = item;
    std::transform(item.begin(), item.end(), ret.begin(), [](auto c) { return std::toupper(c); });
    return ret;
}

#endif // UTIL_H
