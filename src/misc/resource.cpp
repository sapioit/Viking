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

const std::vector<char> &resource::content() const { return _content; }

const fs::path &resource::path() const { return _path; }

const fs::file_time_type &resource::last_write() const { return _last_write; }

resource::resource(const fs::path &path, const std::vector<char> &content)
    : _path(path), _content(content), _last_write(fs::last_write_time(path)) {}

resource::resource(const fs::path &path)
    : _path(path), _content(filesystem::read_file(path)), _last_write(fs::last_write_time(path)) {}

resource::operator bool() { return (_content.size() != 0); }
