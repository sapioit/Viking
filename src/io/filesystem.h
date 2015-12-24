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
