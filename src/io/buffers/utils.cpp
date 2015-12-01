#ifndef UTILS_CPP
#define UTILS_CPP

#include <io/buffers/utils.h>
#include <sys/mman.h>
#include <unistd.h>

std::unique_ptr<MemoryBuffer> From(const UnixFile &file) {
    static constexpr std::size_t max_file_path = 255;
    std::string path;
    path.reserve(max_file_path);

    auto length = file.size - file.offset;
    off64_t pa_offset;
    pa_offset = file.offset & ~(sysconf(_SC_PAGESIZE) - 1);
    auto at = length + file.offset - pa_offset;

    char *const mem_zone = static_cast<char *>(::mmap(NULL, at, PROT_READ, MAP_SHARED, file.fd, file.offset));
    if (mem_zone == MAP_FAILED) {
        throw UnixFile::BadFile{std::addressof(file)};
    }
    const char *const my_thing = mem_zone + file.offset - pa_offset;
    auto result = std::make_unique<MemoryBuffer>(std::vector<char>{my_thing, my_thing + length});
    auto munmap_res = ::munmap(mem_zone, at);
    if (munmap_res != 0) {
        // TODO handle error
    }
    return result;
}

#endif // UTILS_CPP
