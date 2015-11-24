#ifndef SCHEDITEM_H
#define SCHEDITEM_H

#include <vector>
#include <memory>
#include <io/buffers/unix_file.h>
#include <io/buffers/mem_buffer.h>

class ScheduleItem {
    std::vector<std::unique_ptr<DataSource>> buffers;
    bool keep_file_open = false;

    public:
    ScheduleItem() = default;
    virtual ~ScheduleItem() = default;
    ScheduleItem(const ScheduleItem &) = delete;
    ScheduleItem &operator=(const ScheduleItem &) = delete;
    ScheduleItem(ScheduleItem &&) = default;
    ScheduleItem &operator=(ScheduleItem &&) = delete;
    ScheduleItem(bool keep_file_open);
    ScheduleItem(const std::vector<char> &data, bool keep_file_open = true);

    void AddData(std::unique_ptr<MemoryBuffer> data);
    void AddData(std::unique_ptr<UnixFile> file);
    void AddData(ScheduleItem);
    void ReplaceFront(std::unique_ptr<MemoryBuffer>) noexcept;
    inline DataSource *Front() noexcept { return buffers.front().get(); }
    inline void RemoveFront() noexcept { buffers.erase(buffers.begin()); }
    inline bool KeepFileOpen() const noexcept { return keep_file_open; }
    inline void SetKeepFileOpen(bool close) { keep_file_open = close; }
    std::size_t BuffersLeft() const noexcept;
    operator bool() const noexcept;
};

#endif // SCHEDITEM_H
