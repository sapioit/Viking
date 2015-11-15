#ifndef SOCKET_WATCHER_H
#define SOCKET_WATCHER_H

#include <io/watchers/file_watcher.h>
#include <io/socket/socket.h>
#include <io/watchers/sys_epoll.h>
#include <util/set.h>

#include <misc/debug.h>
#include <stdexcept>
#include <algorithm>
#include <utility>

namespace IO
{
template <class Callback> class SocketWatcher : public FileWatcher, public SysEpoll
{
	Callback callback;

      public:
	SocketWatcher() = default;
	virtual std::uint32_t GetBasicFlags() const noexcept
	{
		return (static_cast<std::uint32_t>(SysEpoll::Description::Read) |
			static_cast<std::uint32_t>(SysEpoll::Description::Termination));
	}
    SocketWatcher(Socket sock, Callback callback) : callback(callback)
	{
		try {
            SocketWatcher<Callback>::Add(std::move(sock));
		} catch (const SysEpoll::Error &) {
			throw;
		}
	}
	~SocketWatcher() = default;

    void Add(Socket socket)
	{
		try {
			auto fd = socket.GetFD();
            FileWatcher::Add(std::move(socket));
			SysEpoll::Schedule(fd, GetBasicFlags());
		} catch (const SysEpoll::Error &) {
			throw;
		}
	}

    void Remove(const Socket &socket)
	{
		SysEpoll::Remove(socket.GetFD());
        FileWatcher::Remove(socket);
	}

	virtual void Run() noexcept
	{
        if (FileWatcher::watched_files_.size() == 0)
			return;
		try {
            const auto events = std::move(SysEpoll::Wait(FileWatcher::watched_files_.size()));

            std::vector<std::pair<const Socket *, const Event *>> active_sockets =
			    MatchEventsWithSockets(events);

			for (const auto &event : active_sockets) {
				const auto &associated_socket = *event.first;
				const auto &associated_event = *event.second;

				if (associated_socket.IsAcceptor()) {
					debug("The master socket is active");
					AddNewConnections(associated_socket);
					continue;
				}

				if (associated_event.description &
					static_cast<std::uint32_t>(SysEpoll::Description::Termination) ||
				    associated_event.description &
					static_cast<std::uint32_t>(SysEpoll::Description::Error)) {
					debug("Event with fd = " + std::to_string(associated_event.file_descriptor) +
					      " must be closed");
					Remove(associated_socket);
					continue;
				}
				if (associated_event.description &
				    static_cast<std::uint32_t>(SysEpoll::Description::Read)) {
					debug("Socket with fd = " + std::to_string(associated_socket.GetFD()) +
					      " can be read from");
					bool should_close = callback(associated_socket);
					if (should_close)
						Remove(associated_socket);
				}
			}
		} catch (...) {
			std::rethrow_exception(std::current_exception());
		}
	}

    void AddNewConnections(const Socket &acceptor) noexcept
	{
		while (auto new_connection = acceptor.Accept()) {
			new_connection.MakeNonBlocking();
			debug("Will add a new connection with fd = " + std::to_string(new_connection.GetFD()));
            SocketWatcher<Callback>::Add(std::move(new_connection));
		}
	}

      protected:
    virtual std::vector<std::pair<const Socket *, const Event *>>
	MatchEventsWithSockets(const std::set<SysEpoll::Event> &events)
	{
        std::vector<std::pair<const Socket *, const Event *>> intersection;
        Utility::SetIntersection(FileWatcher::watched_files_.begin(),
                     FileWatcher::watched_files_.end(), events.begin(), events.end(),
                     std::back_inserter(intersection), SysEpoll::EventComparer<Socket>(),
                     Utility::Merge<const Socket, const Event>);
		return intersection;
	}
};
}

#endif
