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
#ifndef UNIX_FILE_H
#define UNIX_FILE_H

#include <functional>
#include <io/buffers/datasource.h>
#include <io/filesystem.h>
#include <string>
#include <sys/types.h>

namespace io {
struct unix_file : public data_source {
    int fd = -1;
    off64_t size = 0;
    off64_t offset = 0;

    public:
    typedef std::function<int(const std::string &)> aquire_func;
    typedef std::function<void(int)> release_func;
    fs::path path;

    private:
    std::function<int(const std::string &)> aquire_func_;
    std::function<void(int)> release_func_;
    void close();

    public:
    struct error {
        std::string path;
        error() = default;
        virtual ~error() = default;
        error(const std::string &path) : path(path) {}
    };
    struct bad_file {
        const unix_file *ptr;
    };

    struct diy {
        const unix_file *ptr;
    };
    struct broken_pipe {
        const unix_file *ptr;
    };

    enum class error_code { none, blocked, broken_pipe, bad_file, diy };

    unix_file() = default;
    virtual ~unix_file();
    unix_file(const std::string &, aquire_func a, release_func r);
    unix_file(unix_file &&);
    unix_file &operator=(unix_file &&);
    unix_file(const unix_file &) = delete;
    unix_file &operator=(const unix_file &) = delete;
    virtual operator bool() const noexcept;
    virtual bool intact() const noexcept;
    std::uint64_t send_to_fd(int, error_code &) noexcept;
    std::uint64_t size_left() const noexcept;
};
}

#endif // UNIX_FILE_H
