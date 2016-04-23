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
#include <algorithm>
#include <io/schedulers/sched_item.h>
#include <typeindex>
using namespace io;
schedule_item::schedule_item() : m_keep_file_open(false) {}

schedule_item::schedule_item(bool keep_file_open) : m_keep_file_open(keep_file_open) {}

schedule_item::schedule_item(const std::vector<char> &data) : m_keep_file_open(false) {
    buffers.push_back(std::make_unique<memory_buffer>(data));
}

schedule_item::schedule_item(const std::vector<char> &data, bool keep_file_open) : m_keep_file_open(keep_file_open) {
    buffers.push_back(std::make_unique<memory_buffer>(data));
}

void schedule_item::put_back(std::unique_ptr<memory_buffer> data) { buffers.push_back(std::move(data)); }

void schedule_item::put_back(std::unique_ptr<unix_file> file) { buffers.push_back(std::move(file)); }

void schedule_item::put_back(schedule_item &&other_item) {
    m_keep_file_open = other_item.m_keep_file_open;
    for (auto &&buffer : other_item.buffers)
        buffers.push_back(std::move(buffer));
}

void schedule_item::put_after_first_intact(std::unique_ptr<memory_buffer> data) { buffers.push_front(std::move(data)); }

void schedule_item::put_after_first_intact(std::unique_ptr<unix_file> file) { buffers.push_front(std::move(file)); }

void schedule_item::put_after_first_intact(schedule_item other_item) {
    buffers.insert(std::find_if(buffers.begin(), buffers.end(), [](auto &ptr) { return ptr->intact(); }),
                   std::make_move_iterator(other_item.buffers.rbegin()),
                   std::make_move_iterator(other_item.buffers.rend()));
}

void schedule_item::replace_front(std::unique_ptr<memory_buffer> with) noexcept { buffers.front() = std::move(with); }

bool schedule_item::is_front_async() const noexcept {
    if (!buffers_left())
        return false;
    const auto &front = *c_front();
    std::type_index type = typeid(front);
    if (type == typeid(memory_buffer) || type == typeid(unix_file))
        return false;
    return true;
}

std::size_t schedule_item::buffers_left() const noexcept { return buffers.size(); }

schedule_item::operator bool() const noexcept { return buffers.size(); }
