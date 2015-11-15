#include <io/schedulers/sched_item.h>

SchedItem::SchedItem(bool close_when_done) : close_when_done(close_when_done) {}

SchedItem::SchedItem(const std::vector<char> &data, bool close_when_done) : close_when_done(close_when_done)
{
	buffers.push_back(std::make_unique<MemoryBuffer>(data));
}

void SchedItem::AddData(std::unique_ptr<MemoryBuffer> data) { buffers.push_back(std::move(data)); }

void SchedItem::AddData(std::unique_ptr<UnixFile> file) { buffers.push_back(std::move(file)); }

void SchedItem::AddData(SchedItem other_item)
{
	for (auto &buffer : other_item.buffers) {
		buffers.push_back(std::move(buffer));
	}
}

void SchedItem::ReplaceFront(std::unique_ptr<MemoryBuffer> with) noexcept { buffers.front() = std::move(with); }

std::size_t SchedItem::BuffersLeft() const noexcept { return buffers.size(); }

SchedItem::operator bool() const noexcept { return buffers.size(); }
