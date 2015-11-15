#include <io/schedulers/io_scheduler.h>

IO::Scheduler::Scheduler(IO::Socket sock, IO::Scheduler::Callback callback) : callback(callback)
{
	try {
		Scheduler::Add(std::move(sock), static_cast<std::uint32_t>(SysEpoll::Description::Read) |
						    static_cast<std::uint32_t>(SysEpoll::Description::Termination));
	} catch (const SysEpoll::Error &) {
		throw;
	}
}

void IO::Scheduler::Add(IO::Socket socket, uint32_t flags)
{
	try {
		auto fd = socket.GetFD();
		SocketContainer::Add(std::move(socket));
		SysEpoll::Schedule(fd, flags);
	} catch (const SysEpoll::Error &) {
		throw;
	}
}

void IO::Scheduler::Remove(const IO::Socket &socket)
{
	schedule_.erase(socket.GetFD());
	SysEpoll::Remove(socket.GetFD());
	SocketContainer::Remove(socket);
}

void IO::Scheduler::Run()
{
	if (SocketContainer::watched_files_.size() == 0)
		return;
	try {
		const auto events = SysEpoll::Wait(SocketContainer::watched_files_.size());
		for (const auto &associated_event : events) {
			const auto &associated_socket = GetSocket(associated_event);

			if (associated_socket.IsAcceptor()) {
				debug("The master socket is active");
				AddNewConnections(associated_socket);
				continue;
			}

			if (CanTerminate(associated_event)) {
				debug("Event with fd = " + std::to_string(associated_event.file_descriptor) +
				      " must be closed");
				Scheduler::Remove(associated_socket);
				continue;
			}
			if (CanWrite(associated_event)) {
				try {
					ProcessWrite(associated_socket, schedule_.at(associated_socket.GetFD()));
				} catch (const DataCorruption &e) {
					Scheduler::Remove(*e.sock);
					debug("Data corruption occured!");
				} catch (const std::out_of_range &) {
					debug("Could not find scheduled item!");
				}
				continue;
			}

			if (CanRead(associated_event)) {
				debug("Socket with fd = " + std::to_string(associated_socket.GetFD()) +
				      " can be read from, and there is no write scheduled for it");
				CallbackResponse callback_response = callback(associated_socket);
				if (callback_response) {
					/* Schedule the item in the epoll instance with just the Write flag,
					     * since it already has the others
					     */
					Modify(associated_event.file_descriptor,
					       static_cast<std::uint32_t>(SysEpoll::Description::Write));
					AddSchedItem(associated_event, callback_response);
				}
				continue;
			}
		}
	} catch (...) {
		std::rethrow_exception(std::current_exception());
	}
}

void IO::Scheduler::AddSchedItem(const SysEpoll::Event &ev, const IO::Scheduler::SchedItem &item, bool append) noexcept
{
	auto item_it = schedule_.find(ev.file_descriptor);
	if (item_it == schedule_.end()) {
		schedule_.emplace(std::make_pair(ev.file_descriptor, item.data));
	} else {
		auto &sched_item_data = item_it->second.data;
		if (append) {
			sched_item_data.reserve(sched_item_data.size() + item.data.size());
			sched_item_data.insert(sched_item_data.end(), item.data.begin(), item.data.end());
		} else {
			sched_item_data = std::move(item.data);
		}
	}
}

void IO::Scheduler::ProcessWrite(const IO::Socket &socket, IO::Scheduler::SchedItem &sched_item)
{
	auto &data = sched_item.data;
	auto written = socket.WriteSome(data);

	if (written == data.size()) {
		ScheduledItemFinished(socket, sched_item);
	} else {
		auto old_data_size = data.size();
		DataType(data.begin() + written, data.end()).swap(data);
		if (old_data_size - written != data.size())
			throw DataCorruption{std::addressof(socket)};
	}
}

bool IO::Scheduler::CanWrite(const SysEpoll::Event &event) const noexcept
{
	return (event.description & static_cast<std::uint32_t>(SysEpoll::Description::Write));
}

bool IO::Scheduler::CanRead(const SysEpoll::Event &event) const noexcept
{
	return (event.description & static_cast<std::uint32_t>(SysEpoll::Description::Read));
}

bool IO::Scheduler::IsScheduled(const SysEpoll::Event &event) const noexcept
{
	auto item = schedule_.find(event.file_descriptor);
	return (item != schedule_.end());
}

bool IO::Scheduler::CanTerminate(const SysEpoll::Event &event) const noexcept
{
	return (event.description & static_cast<std::uint32_t>(SysEpoll::Description::Termination) ||
		event.description & static_cast<std::uint32_t>(SysEpoll::Description::Error));
}

void IO::Scheduler::AddNewConnections(const IO::Socket &acceptor) noexcept
{
	while (auto new_connection = acceptor.Accept()) {
		new_connection.MakeNonBlocking();
		Scheduler::Add(std::move(new_connection),
			       static_cast<std::uint32_t>(SysEpoll::Description::Read) |
				   static_cast<std::uint32_t>(SysEpoll::Description::Termination));
	}
}

const IO::Socket &IO::Scheduler::GetSocket(const SysEpoll::Event &event) const
{
	for (auto it = watched_files_.cbegin(); it != watched_files_.cend(); ++it) {
		if ((*it).GetFD() == event.file_descriptor) {
			return *it;
		}
	}
	throw SocketNotFound{std::addressof(event)};
}
