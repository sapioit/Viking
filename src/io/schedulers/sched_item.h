#ifndef SCHEDITEM_H
#define SCHEDITEM_H

#include <vector>
#include <memory>
#include <io/buffers/unix_file.h>
#include <io/buffers/mem_buffer.h>

class SchedItem
{
	std::vector<std::unique_ptr<DataSource>> buffers;
	bool close_when_done = true;

	public:
	SchedItem() = default;
	virtual ~SchedItem() = default;
	SchedItem(const SchedItem &) = delete;
	SchedItem &operator=(const SchedItem &) = delete;
	SchedItem(SchedItem &&) = default;
	SchedItem &operator=(SchedItem &&) = delete;
	SchedItem(bool close_when_done);
	SchedItem(const std::vector<char> &data, bool close_when_done = true);

	void AddData(std::unique_ptr<MemoryBuffer> data);
	void AddData(std::unique_ptr<UnixFile> file);
	void AddData(SchedItem);
	void ReplaceFront(std::unique_ptr<MemoryBuffer>) noexcept;
	inline DataSource *Front() noexcept { return buffers.front().get(); }
	inline void RemoveFront() noexcept { buffers.erase(buffers.begin()); }
	inline bool CloseWhenDone() const noexcept { return close_when_done; }
	std::size_t BuffersLeft() const noexcept;
	operator bool() const noexcept;
};

#endif // SCHEDITEM_H
