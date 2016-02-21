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
#ifndef UTILS_CPP
#define UTILS_CPP

#include <io/buffers/utils.h>
#include <sys/mman.h>
#include <unistd.h>

std::unique_ptr<io::memory_buffer> from(const io::unix_file &file) {
    static constexpr std::size_t max_file_path = 255;
    std::string path;
    path.reserve(max_file_path);

    auto length = file.size - file.offset;
    off64_t pa_offset;
    pa_offset = file.offset & ~(sysconf(_SC_PAGESIZE) - 1);
    auto at = length + file.offset - pa_offset;

    char *const mem_zone = static_cast<char *>(::mmap(NULL, at, PROT_READ, MAP_SHARED, file.fd, file.offset));
    if (mem_zone == MAP_FAILED)
        throw io::unix_file::bad_file{std::addressof(file)};
    const char *const my_thing = mem_zone + file.offset - pa_offset;
    auto result = std::make_unique<io::memory_buffer>(std::vector<char>{my_thing, my_thing + length});
    ::munmap(mem_zone, at);
    return result;
}

#endif // UTILS_CPP
