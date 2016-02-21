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
#ifndef MEMBUFFER_H
#define MEMBUFFER_H

#include <io/buffers/datasource.h>

#include <vector>

namespace io {
struct memory_buffer : public data_source {
    std::vector<char> data;
    std::size_t initial_size;

    memory_buffer(const std::vector<char> &data) : data(data), initial_size(data.size()) {}
    virtual operator bool() const noexcept { return data.size() != 0; }
    virtual bool intact() const noexcept { return data.size() == initial_size; }
};
}

#endif // MEMBUFFER_H
