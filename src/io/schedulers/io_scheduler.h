#ifndef SOCKET_WATCHER_H
#define SOCKET_WATCHER_H

#include <io/schedulers/sys_epoll.h>
#include <io/schedulers/sched_item.h>
#include <io/schedulers/channel.h>
#include <io/socket/socket.h>

#include <misc/debug.h>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <map>
#include <functional>

namespace IO {
class Scheduler {
    private:
    struct SocketNotFound {
        const SysEpoll::Event *event;
    };

    public:
    typedef ScheduleItem Resolution;
    typedef std::vector<char> DataType;
    typedef std::function<Resolution(const Socket *)> Callback;

    private:
    std::map<int, ScheduleItem> schedule_;
    std::vector<std::unique_ptr<Channel>> contexts_;
    SysEpoll poller_;
    Callback callback;

    public:
    Scheduler() = default;

    Scheduler(std::unique_ptr<Socket> sock, Callback callback);
    ~Scheduler() = default;

    void Add(std::unique_ptr<Socket> socket, std::uint32_t flags);

    virtual void Run();

    protected:
    void Remove(const Channel *);

    void AddSchedItem(const SysEpoll::Event &ev, ScheduleItem item, bool append = true) noexcept;

    void ScheduledItemFinished(const Channel *, ScheduleItem &sched_item);

    void ProcessWrite(const Channel *socket, ScheduleItem &sched_item);

    inline bool CanWrite(const SysEpoll::Event &event) const noexcept;

    inline bool CanRead(const SysEpoll::Event &event) const noexcept;

    inline bool HasDataScheduled(int) const noexcept;

    inline bool CanTerminate(const SysEpoll::Event &event) const noexcept;

    void AddNewConnections(const Channel *) noexcept;
};
}

#endif
