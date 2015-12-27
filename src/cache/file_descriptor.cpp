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
#include <map>
#include <algorithm>
#include <utility>
#include <mutex>

struct handle_use_count {
    std::size_t use_count;
    int handle;
    bool operator==(const handle_use_count &other) {
        return (handle == other.handle) && (use_count == other.use_count);
    }
};

static std::map<fs::path, handle_use_count> cache;
static std::mutex m;

int Cache::FileDescriptor::Aquire(const std::string &path) noexcept {
    std::lock_guard<std::mutex> hold(m);

    auto it = cache.find(path);
    if (it != cache.end()) {
        ++(it->second.use_count);
        return it->second.handle;
    } else {
        int fd = ::open(path.c_str(), O_RDONLY);
        if (fd != -1) {
            cache.emplace(std::make_pair(path, handle_use_count{1, fd}));
            return fd;
        }
    }
    return -1;
}

void Cache::FileDescriptor::Release(int file_descriptor) noexcept {
    std::lock_guard<std::mutex> hold(m);

    auto it = std::find_if(cache.begin(), cache.end(),
                           [file_descriptor](auto &pair) { return file_descriptor == pair.second.handle; });
    if (it != cache.end()) {
        --(it->second.use_count);
        if (it->second.use_count == 0) {
            ::close(it->second.handle);
            cache.erase(it->first);
        }
    }
}
