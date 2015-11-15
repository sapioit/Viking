#ifndef SOCKET_WATCHER_H
#define SOCKET_WATCHER_H

#include <io/schedulers/file_container.h>
#include <io/schedulers/sys_epoll.h>
#include <io/socket/socket.h>
#include <util/set.h>

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
	typedef std::vector<char> DataType;

	private:
	struct SchedItem {
		DataType data;
		bool close_when_done = true;
		SchedItem() = default;
		SchedItem(const DataType &data, bool close_when_done = true) try : data(data),
										   close_when_done(close_when_done) {
		} catch (...) {
			throw;
		}
		operator bool() { return data.size(); }
	};
	struct DataCorruption {
		const Socket *sock;
	};
	struct SocketNotFound {
		const SysEpoll::Event *event;
	};

	public:
	typedef SchedItem CallbackResponse;
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
	void AddSchedItem(const SysEpoll::Event &ev, const SchedItem &item, bool append = true) noexcept;

	void ScheduledItemFinished(const Socket &socket, SchedItem &sched_item)
	{
		if (sched_item.close_when_done) {
			Scheduler::Remove(socket);
		} else {
			schedule_.erase(socket.GetFD());
		}
	}

	void ProcessWrite(const Socket &socket, SchedItem &sched_item);

	inline bool CanWrite(const Event &event) const noexcept;

	inline bool CanRead(const Event &event) const noexcept;

	inline bool IsScheduled(const Event &event) const noexcept;

	inline bool CanTerminate(const Event &event) const noexcept;

	void AddNewConnections(const Socket &acceptor) noexcept;

	const Socket &GetSocket(const SysEpoll::Event &event) const;
};
}

#endif
