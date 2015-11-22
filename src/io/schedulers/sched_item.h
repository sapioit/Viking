#ifndef SCHEDITEM_H
#define SCHEDITEM_H

#include <vector>
#include <memory>
#include <io/buffers/unix_file.h>
#include <io/buffers/mem_buffer.h>

class ScheduleItem
{
	std::vector<std::unique_ptr<DataSource>> buffers;
	bool close_when_done = true;

	public:
    ScheduleItem() = default;
    virtual ~ScheduleItem() = default;
    ScheduleItem(const ScheduleItem &) = delete;
    ScheduleItem &operator=(const ScheduleItem &) = delete;
    ScheduleItem(ScheduleItem &&) = default;
    ScheduleItem &operator=(ScheduleItem &&) = delete;
    ScheduleItem(bool close_when_done);
    ScheduleItem(const std::vector<char> &data, bool close_when_done = true);

	void AddData(std::unique_ptr<MemoryBuffer> data);
	void AddData(std::unique_ptr<UnixFile> file);
    void AddData(ScheduleItem);
	void ReplaceFront(std::unique_ptr<MemoryBuffer>) noexcept;
	inline DataSource *Front() noexcept { return buffers.front().get(); }
	inline void RemoveFront() noexcept { buffers.erase(buffers.begin()); }
	inline bool CloseWhenDone() const noexcept { return close_when_done; }
	std::size_t BuffersLeft() const noexcept;
	operator bool() const noexcept;
};

#endif // SCHEDITEM_H
