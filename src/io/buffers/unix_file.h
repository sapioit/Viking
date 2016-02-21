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

#include <io/buffers/datasource.h>

#include <sys/types.h>
#include <functional>
#include <string>

namespace IO {
struct unix_file : public data_source {
    int fd = -1;
    off64_t size = 0;
    off64_t offset = 0;

    public:
    typedef std::function<int(const std::string &)> AquireFunction;
    typedef std::function<void(int)> ReleaseFunction;

    private:
    std::function<int(const std::string &)> aquire_func_;
    std::function<void(int)> release_func_;
    void close();

    public:
    struct Error {
        std::string path;
        Error() = default;
        virtual ~Error() = default;
        Error(const std::string &path) : path(path) {}
    };
    struct BadFile {
        const unix_file *ptr;
    };

    struct DIY {
        const unix_file *ptr;
    };
    struct BrokenPipe {
        const unix_file *ptr;
    };

    unix_file() = default;
    virtual ~unix_file();
    unix_file(const std::string &, AquireFunction a, ReleaseFunction r);
    unix_file(unix_file &&);
    unix_file &operator=(unix_file &&);
    unix_file(const unix_file &) = delete;
    unix_file &operator=(const unix_file &) = delete;
    virtual operator bool() const noexcept;
    virtual bool Intact() const noexcept;
    std::uint64_t SendTo(int);
    std::uint64_t SizeLeft() const noexcept;
};
}

#endif // UNIX_FILE_H
