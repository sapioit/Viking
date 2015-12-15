#ifndef SCHEDITEM_H
#define SCHEDITEM_H

#include <deque>
#include <memory>
#include <io/buffers/unix_file.h>
#include <io/buffers/mem_buffer.h>
#include <io/buffers/asyncbuffer.h>

class ScheduleItem {
    std::deque<std::unique_ptr<DataSource>> buffers;
    bool keep_file_open = false;

    auto GetFirstIntact() {
        for (auto it = buffers.begin(); it != buffers.end(); ++it)
            if (it->get()->Intact())
                return it;
        return buffers.end();
    }

    public:
    ScheduleItem() = default;
    virtual ~ScheduleItem() = default;
    ScheduleItem(const ScheduleItem &) = delete;
    ScheduleItem &operator=(const ScheduleItem &) = delete;
    ScheduleItem(ScheduleItem &&) = default;
    ScheduleItem &operator=(ScheduleItem &&) = delete;
    ScheduleItem(bool keep_file_open);
    ScheduleItem(const std::vector<char> &data);
    ScheduleItem(const std::vector<char> &data, bool);

    template <typename T> ScheduleItem(std::unique_ptr<AsyncBuffer<T>> future) {
        buffers.emplace_back(std::move(future));
    }

    void PutBack(std::unique_ptr<MemoryBuffer> data);
    void PutBack(std::unique_ptr<UnixFile> file);
    void PutBack(ScheduleItem);

    void PutAfterFirstIntact(std::unique_ptr<MemoryBuffer> data);
    void PutAfterFirstIntact(std::unique_ptr<UnixFile> file);
    void PutAfterFirstIntact(ScheduleItem);

    void ReplaceFront(std::unique_ptr<MemoryBuffer>) noexcept;
    inline DataSource *Front() noexcept { return buffers.front().get(); }
    inline const DataSource *CFront() const noexcept { return buffers.front().get(); }
    bool IsFrontAsync() const noexcept;
    inline void RemoveFront() noexcept { buffers.erase(buffers.begin()); }
    inline bool KeepFileOpen() const noexcept { return keep_file_open; }
    inline void SetKeepFileOpen(bool close) { keep_file_open = close; }
    std::size_t BuffersLeft() const noexcept;
    operator bool() const noexcept;
};

#endif // SCHEDITEM_H
