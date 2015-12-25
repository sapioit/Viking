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
#ifndef FILE_DESCRIPTOR_H
#define FILE_DESCRIPTOR_H

#include <unordered_map>
#include <string>

namespace Cache {
class FileDescriptor {
    struct handle_use_count {
        std::size_t use_count;
        int handle;
        bool operator==(const handle_use_count &other) {
            return (handle == other.handle) && (use_count == other.use_count);
        }
    };

    static std::unordered_map<std::string, handle_use_count> file_descriptor_cache_;

    public:
    static int Aquire(const std::string &) noexcept;
    static void Release(int) noexcept;
};
}

#endif
