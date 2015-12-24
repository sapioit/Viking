#include "unix_file.h"

#include <sys/stat.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <fcntl.h>
#include <io/filesystem.h>

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

bool UnixFile::Intact() const noexcept { return offset == 0; }

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

std::uint64_t UnixFile::SendTo(int other_file) {
    const ssize_t ret = ::sendfile64(other_file, fd, std::addressof(offset), SizeLeft());
    if (ret == -1) {
        switch (errno) {
        case EAGAIN:
            return 0;
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
    return ret;
}

std::uint64_t UnixFile::SizeLeft() const noexcept { return static_cast<std::size_t>(size - offset); }
