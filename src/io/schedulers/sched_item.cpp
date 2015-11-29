#include <io/schedulers/sched_item.h>
#include <algorithm>

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

std::size_t ScheduleItem::BuffersLeft() const noexcept { return buffers.size(); }

ScheduleItem::operator bool() const noexcept { return buffers.size(); }
