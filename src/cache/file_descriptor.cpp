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
#include <cache/file_descriptor.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <utility>

std::unordered_map<std::string, Cache::FileDescriptor::handle_use_count> Cache::FileDescriptor::file_descriptor_cache_;

int Cache::FileDescriptor::Aquire(const std::string &path) noexcept {
    auto it = file_descriptor_cache_.find(path);
    if (it != file_descriptor_cache_.end()) {
        ++(it->second.use_count);
        return it->second.handle;
    } else {
        int fd = ::open(path.c_str(), O_RDONLY);
        if (fd != -1) {
            file_descriptor_cache_.emplace(std::make_pair(path, Cache::FileDescriptor::handle_use_count{1, fd}));
            return fd;
        }
    }
    return -1;
}

void Cache::FileDescriptor::Release(int file_descriptor) noexcept {
    auto it = std::find_if(file_descriptor_cache_.begin(), file_descriptor_cache_.end(),
                           [file_descriptor](auto &pair) { return file_descriptor == pair.second.handle; });
    if (it != file_descriptor_cache_.end()) {
        --(it->second.use_count);
        if (it->second.use_count == 0) {
            ::close(it->second.handle);
            file_descriptor_cache_.erase(it->first);
        }
    }
}
