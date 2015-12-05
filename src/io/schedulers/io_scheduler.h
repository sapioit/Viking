#ifndef SOCKET_WATCHER_H
#define SOCKET_WATCHER_H

#include <io/schedulers/sys_epoll.h>
#include <io/schedulers/sched_item.h>
#include <io/socket/socket.h>
#include <io/buffers/mem_buffer.h>
#include <map>
#include <functional>

class Channel;

namespace IO {
class Scheduler {
    private:
    struct SocketNotFound {
        const SysEpoll::Event *event;
    };
    struct WriteError {};

    public:
    typedef ScheduleItem Resolution;
    typedef std::vector<char> DataType;
    typedef std::function<Resolution(const Socket *)> ReadCallback;
    typedef std::function<bool(ScheduleItem &, std::unique_ptr<MemoryBuffer> &)> BarrierCallback;

    private:
    std::map<int, ScheduleItem> schedule_map_;
    std::vector<std::unique_ptr<Channel>> contexts_;
    SysEpoll poller_;
    ReadCallback read_callback;
    BarrierCallback barrier_callback;

    public:
    Scheduler() = default;
    Scheduler(std::unique_ptr<Socket> sock, ReadCallback, BarrierCallback);
    ~Scheduler() = default;

    void Add(std::unique_ptr<Socket> socket, std::uint32_t flags);
    virtual void Run() noexcept;

    protected:
    void AddSchedItem(const SysEpoll::Event &, ScheduleItem, bool = true) noexcept;
    void AddNewConnections(const Channel *) noexcept;
    void Remove(Channel *) noexcept;
    void ProcessWrite(Channel *socket) noexcept;
    void ConsumeItem(ScheduleItem &, Channel *channel);
    inline bool CanWrite(const SysEpoll::Event &event) const noexcept;
    inline bool CanRead(const SysEpoll::Event &event) const noexcept;
    inline bool HasDataScheduled(int) const noexcept;
    inline bool CanTerminate(const SysEpoll::Event &event) const noexcept;
};
}

#endif
