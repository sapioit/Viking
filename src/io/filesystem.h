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

namespace IO {

struct fs_error : public std::runtime_error {
    fs_error() = default;
    fs_error(const std::string &msg) : std::runtime_error(msg) {}

    ~fs_error() = default;
};

class FileSystem {
    public:
    static std::vector<char> ReadFile(const std::string &path);
    static std::string GetCurrentDirectory();
    static std::size_t GetFileSize(const std::string &file_path);
    static bool FileExists(const std::string &) noexcept;
    static bool IsRegularFile(const std::string &) noexcept;
    static std::string GetExtension(const std::string &) noexcept;
};
};

#endif // FILE_H
