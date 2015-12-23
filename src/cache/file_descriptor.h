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
