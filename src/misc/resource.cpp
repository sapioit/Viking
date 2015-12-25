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
#include <misc/resource.h>
#include <io/filesystem.h>

const std::vector<char> &Resource::content() const { return _content; }

const std::string &Resource::path() const { return _path; }

struct stat Resource::stat() const {
    return _stat;
}
Resource::Resource(const std::string &path, const std::vector<char> &content, const struct stat &stat)
    : _path(path), _content(content), _stat(stat) {}

Resource::Resource(const std::string &path) : _path(path) {
    try {
        _content = IO::FileSystem::ReadFile(path);
        auto stat_res = ::stat(_path.c_str(), &_stat);
        if (stat_res != 0)
            throw IO::fs_error("Could not stat " + path);
    } catch (IO::fs_error &ex) {
        throw;
    }
}

Resource::operator bool() { return (_content.size() != 0); }
