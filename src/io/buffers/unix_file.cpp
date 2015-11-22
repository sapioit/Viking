#include "unix_file.h"

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <io/filesystem.h>
#include <misc/storage.h>

void UnixFile::Close() {
    if (-1 != fd) {
        release_func_(fd);
    }
}

UnixFile::~UnixFile() { Close(); }

UnixFile::UnixFile(UnixFile &&other) : fd(-1) { *this = std::move(other); }

UnixFile &UnixFile::operator=(UnixFile &&other) {
    if (this != &other) {
        Close();
        fd = other.fd;
        other.fd = -1;
        offset = other.offset;
        other.offset = 0;
    }
    return *this;
}

UnixFile::operator bool() const noexcept { return !(offset == size); }

UnixFile::UnixFile(const std::string &path, AquireFunction a, ReleaseFunction r) : aquire_func_(a), release_func_(r) {
    fd = aquire_func_(path);
    if (-1 == fd)
        throw Error{path};
    struct stat64 stat;
    if (-1 == ::stat64(path.c_str(), &stat)) {
        throw Error{path};
    }
    size = stat.st_size;
}

bool UnixFile::SendTo(int other_file) {
    auto size_left = static_cast<std::size_t>(size - offset);
    const int ret = ::sendfile64(other_file, fd, std::addressof(offset), size_left);
    if (ret == -1) {
        switch (errno) {
        case EAGAIN:
            return false;
        case EINVAL:
            throw DIY{this};
            break;
        case EBADF:
            throw BadFile{this};
            break;
        case EIO:
            throw BadFile{this};
            break;
        case EFAULT:
            // WTF
            break;
        case EPIPE:
            throw BrokenPipe{this};
        default:
            break;
        }
    }
    return true;
}
