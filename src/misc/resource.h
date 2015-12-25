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

#include <cstdint>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

class Resource {
    std::string _path;
    std::vector<char> _content;
    // TODO Modify to use the stat64 version
    struct stat _stat;

    public:
    Resource() = default;
    Resource(const Resource &) = default;
    Resource(const std::string &, const std::vector<char> &, const struct stat &);
    Resource(const std::string &);
    ~Resource() = default;
    operator bool();
    bool operator<(const Resource &);

    const std::uint64_t &hits() const;
    const std::vector<char> &content() const;
    const std::string &path() const;
    struct stat stat() const;
};

#endif // RESOURCE_H
