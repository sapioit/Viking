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
#ifndef SCHEDITEM_H
#define SCHEDITEM_H

#include <deque>
#include <memory>
#include <io/buffers/unix_file.h>
#include <io/buffers/mem_buffer.h>
#include <io/buffers/asyncbuffer.h>

class schedule_item {
    std::deque<std::unique_ptr<data_source>> buffers;
    bool m_keep_file_open;

    auto GetFirstIntact() {
        for (auto it = buffers.begin(); it != buffers.end(); ++it)
            if (it->get()->intact())
                return it;
        return buffers.end();
    }

    public:
    schedule_item();
    virtual ~schedule_item() = default;
    schedule_item(const schedule_item &) = delete;
    schedule_item &operator=(const schedule_item &) = delete;
    schedule_item(schedule_item &&) = default;
    schedule_item &operator=(schedule_item &&) = delete;
    schedule_item(bool keep_file_open);
    explicit schedule_item(const std::vector<char> &data);
    schedule_item(const std::vector<char> &data, bool);

    template <typename T> schedule_item(std::unique_ptr<AsyncBuffer<T>> future) {
        buffers.emplace_back(std::move(future));
    }

    void put_back(std::unique_ptr<io::memory_buffer> data);
    void put_back(std::unique_ptr<io::unix_file> file);
    void put_back(schedule_item &&);

    void put_after_first_intact(std::unique_ptr<io::memory_buffer> data);
    void put_after_first_intact(std::unique_ptr<io::unix_file> file);
    void put_after_first_intact(schedule_item);

    void replace_front(std::unique_ptr<io::memory_buffer>) noexcept;
    inline data_source *front() noexcept { return buffers.front().get(); }
    inline const data_source *c_front() const noexcept { return buffers.front().get(); }
    bool is_front_async() const noexcept;
    inline void remove_front() noexcept { buffers.erase(buffers.begin()); }
    inline bool keep_file_open() const noexcept { return this->m_keep_file_open; }
    inline void set_keep_file_open(bool close) { this->m_keep_file_open = close; }
    std::size_t buffers_left() const noexcept;
    operator bool() const noexcept;
};

#endif // SCHEDITEM_H
