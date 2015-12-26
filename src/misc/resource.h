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
#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>
#include <vector>
#include <io/filesystem.h>

class Resource {
    fs::path _path;
    std::vector<char> _content;
    fs::file_time_type _last_write;

    public:
    Resource() = default;
    Resource(const fs::path &, const std::vector<char> &);
    Resource(const fs::path &);
    ~Resource() = default;
    operator bool();

    const std::vector<char> &Content() const;
    const fs::path &Path() const;
    const fs::file_time_type &LastWrite() const;
};

#endif // RESOURCE_H
