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
