#include <io/schedulers/sched_item.h>

ScheduleItem::ScheduleItem(bool close_when_done) : close_when_done(close_when_done) {}

ScheduleItem::ScheduleItem(const std::vector<char> &data, bool close_when_done) : close_when_done(close_when_done) {
    buffers.push_back(std::make_unique<MemoryBuffer>(data));
}

void ScheduleItem::AddData(std::unique_ptr<MemoryBuffer> data) { buffers.push_back(std::move(data)); }

void ScheduleItem::AddData(std::unique_ptr<UnixFile> file) { buffers.push_back(std::move(file)); }

void ScheduleItem::AddData(ScheduleItem other_item) {
    for (auto &buffer : other_item.buffers) {
        buffers.push_back(std::move(buffer));
    }
}

void ScheduleItem::ReplaceFront(std::unique_ptr<MemoryBuffer> with) noexcept { buffers.front() = std::move(with); }

std::size_t ScheduleItem::BuffersLeft() const noexcept { return buffers.size(); }

ScheduleItem::operator bool() const noexcept { return buffers.size(); }
