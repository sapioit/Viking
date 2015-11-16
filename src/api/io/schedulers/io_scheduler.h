#ifndef SOCKET_WATCHER_H
#define SOCKET_WATCHER_H

#include <io/schedulers/file_container.h>
#include <io/schedulers/sys_epoll.h>
#include <io/schedulers/sched_item.h>
#include <io/socket/socket.h>

#include <misc/debug.h>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <map>
#include <functional>

namespace IO
{
class Scheduler : public SocketContainer, public SysEpoll
{
	public:
	private:
	struct DataCorruption {
		const Socket *sock;
	};
	struct SocketNotFound {
		const SysEpoll::Event *event;
	};

	public:
	typedef SchedItem CallbackResponse;
	typedef SchedItem ScheduleItem;
	typedef std::vector<char> DataType;
	typedef std::function<CallbackResponse(const Socket &)> Callback;

	private:
	std::map<int, SchedItem> schedule_;
	Callback callback;

	public:
	Scheduler() = default;

	Scheduler(Socket sock, Callback callback);
	~Scheduler() = default;

	void Add(Socket socket, std::uint32_t flags);

	void Remove(const Socket &socket);

	virtual void Run();

	protected:
	void AddSchedItem(const SysEpoll::Event &ev, ScheduleItem item, bool append = true) noexcept;

	void ScheduledItemFinished(const Socket &socket, SchedItem &sched_item);

	void ProcessWrite(const Socket &socket, SchedItem &sched_item);

	inline bool CanWrite(const Event &event) const noexcept;

	inline bool CanRead(const Event &event) const noexcept;

    inline bool IsScheduled(int) const noexcept;

	inline bool CanTerminate(const Event &event) const noexcept;

	void AddNewConnections(const Socket &acceptor) noexcept;

	const Socket &GetSocket(const SysEpoll::Event &event) const;
};
}

#endif
