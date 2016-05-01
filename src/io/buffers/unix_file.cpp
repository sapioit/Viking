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
#include <fcntl.h>
#include <io/buffers/unix_file.h>
#include <io/filesystem.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace io;

void unix_file::close() {
    if (-1 != fd) {
        release_func_(fd);
    }
    if (mem_mapping) {
        munmap(mem_mapping, size);
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
        mem_mapping = other.mem_mapping;
        other.mem_mapping = nullptr;
    }
    return *this;
}

bool unix_file::intact() const noexcept { return offset == 0; }

unix_file::operator bool() const noexcept { return !(offset == size); }

unix_file::unix_file(const std::string &path, aquire_func a, release_func r)
    : path(path), aquire_func_(a), release_func_(r) {
    fd = aquire_func_(path);
    size = fs::file_size(path);
    mem_mapping = (char *)::mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
    if ((void *)mem_mapping == (void *)MAP_FAILED)
        mem_mapping = nullptr;
    else
        madvise(mem_mapping, size, MADV_SEQUENTIAL | MADV_WILLNEED);
}

std::uint64_t unix_file::send_to_fd(int other_file, error_code &ec) noexcept {
    ec = error_code::none;
    const ssize_t sent = ::sendfile64(other_file, fd, std::addressof(offset), size_left());
    if (sent == -1) {
        switch (errno) {
        case EAGAIN:
            ec = error_code::blocked;
            break;
        case EINVAL:
            ec = error_code::diy;
            break;
        case EBADF:
        case EIO:
            ec = error_code::bad_file;
            break;
        case EPIPE:
            ec = error_code::broken_pipe;
        default:
            break;
        }
    }
    return sent >= 0 ? sent : 0;
}

std::uint64_t unix_file::size_left() const noexcept { return static_cast<std::size_t>(size - offset); }
