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
#ifndef FILE_H
#define FILE_H

#include <vector>
#include <string>
#include <stdexcept>
#include <experimental/filesystem>

static constexpr auto fs_lib_v() {
#ifdef __cpp_lib_experimental_filesystem
    return __cpp_lib_experimental_filesystem;
#else
    return 0;
#endif
}

static_assert(fs_lib_v(), "You need filesystem support in your standard "
                          "library (std::experimental::filesystem");

namespace fs = std::experimental::filesystem;

namespace io {
std::vector<char> read_file(const fs::path &path);
std::string get_extension(const std::string &) noexcept;
}

#endif // FILE_H
