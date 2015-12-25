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
#include <io/schedulers/sched_item.h>
#include <algorithm>
#include <typeindex>

ScheduleItem::ScheduleItem(bool keep_file_open) : keep_file_open(keep_file_open) {}

ScheduleItem::ScheduleItem(const std::vector<char> &data) { buffers.push_back(std::make_unique<MemoryBuffer>(data)); }

ScheduleItem::ScheduleItem(const std::vector<char> &data, bool keep_file_open) : keep_file_open(keep_file_open) {
    buffers.push_back(std::make_unique<MemoryBuffer>(data));
}

void ScheduleItem::PutBack(std::unique_ptr<MemoryBuffer> data) { buffers.push_back(std::move(data)); }

void ScheduleItem::PutBack(std::unique_ptr<UnixFile> file) { buffers.push_back(std::move(file)); }

void ScheduleItem::PutBack(ScheduleItem other_item) {
    for (auto &buffer : other_item.buffers)
        buffers.push_back(std::move(buffer));
}

void ScheduleItem::PutAfterFirstIntact(std::unique_ptr<MemoryBuffer> data) { buffers.push_front(std::move(data)); }

void ScheduleItem::PutAfterFirstIntact(std::unique_ptr<UnixFile> file) { buffers.push_front(std::move(file)); }

void ScheduleItem::PutAfterFirstIntact(ScheduleItem other_item) {
    buffers.insert(std::find_if(buffers.begin(), buffers.end(), [](auto &ptr) { return ptr->Intact(); }),
                   std::make_move_iterator(other_item.buffers.rbegin()),
                   std::make_move_iterator(other_item.buffers.rend()));
}

void ScheduleItem::ReplaceFront(std::unique_ptr<MemoryBuffer> with) noexcept { buffers.front() = std::move(with); }

bool ScheduleItem::IsFrontAsync() const noexcept {
    if (!BuffersLeft())
        return false;
    const auto &front = *CFront();
    std::type_index type = typeid(front);
    if (type == typeid(MemoryBuffer) || type == typeid(UnixFile))
        return false;
    return true;
}

std::size_t ScheduleItem::BuffersLeft() const noexcept { return buffers.size(); }

ScheduleItem::operator bool() const noexcept { return buffers.size(); }
