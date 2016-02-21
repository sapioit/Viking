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
#include <io/buffers/unix_file.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <fcntl.h>
#include <io/filesystem.h>

using namespace io;

void unix_file::close() {
    if (-1 != fd) {
        release_func_(fd);
    }
}

unix_file::~unix_file() { close(); }

unix_file::unix_file(unix_file &&other) : fd(-1) { *this = std::move(other); }

unix_file &unix_file::operator=(unix_file &&other) {
    if (this != &other) {
        close();
        fd = other.fd;
        other.fd = -1;
        offset = other.offset;
        other.offset = 0;
    }
    return *this;
}

bool unix_file::intact() const noexcept { return offset == 0; }

unix_file::operator bool() const noexcept { return !(offset == size); }

unix_file::unix_file(const std::string &path, aquire_func a, release_func r) : aquire_func_(a), release_func_(r) {
    fd = aquire_func_(path);
    if (-1 == fd)
        throw error{path};
    struct stat64 stat;
    if (-1 == ::stat64(path.c_str(), &stat)) {
        throw error{path};
    }
    size = stat.st_size;
}

std::uint64_t unix_file::send_to_fd(int other_file) {
    const ssize_t ret = ::sendfile64(other_file, fd, std::addressof(offset), size_left());
    if (ret == -1) {
        switch (errno) {
        case EAGAIN:
            return 0;
        case EINVAL:
            throw diy{this};
            break;
        case EBADF:
            throw bad_file{this};
            break;
        case EIO:
            throw bad_file{this};
            break;
        case EFAULT:
            // WTF
            break;
        case EPIPE:
            throw broken_pipe{this};
        default:
            break;
        }
    }
    return ret;
}

std::uint64_t unix_file::size_left() const noexcept { return static_cast<std::size_t>(size - offset); }
