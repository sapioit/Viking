#ifndef OUT_H
#define OUT_H
#include <io/watchers/socket_watcher.h>
#include <util/set.h>
#include <misc/debug.h>

#include <map>
#include <mutex>
#include <cassert>
#include <execinfo.h>

namespace IO
{
namespace Scheduler
{

template <class Data> struct SchedItem {
	Data data;
	SchedItem(const Data &data) try : data(data) {
	} catch (...) {
		throw;
	}
};

template <class Sock, class Data> class Out : public SocketWatcher<Sock, std::function<bool(const Sock &)>>
{
	typedef std::function<bool(const Sock &)> Callback;
	typedef SocketWatcher<Sock, Callback> Base;

	std::map<int, SchedItem<Data>> schedule_;
	std::mutex sync_;
	typedef std::lock_guard<std::mutex> Lock;
	static std::unique_ptr<Out> instance_;
	struct DataCorruption {
		const Sock *sock;
	};

      public:
	virtual std::uint32_t GetBasicFlags() const noexcept
	{
		return (static_cast<std::uint32_t>(SysEpoll::Description::Write) |
			static_cast<std::uint32_t>(SysEpoll::Description::Termination));
	}

	Out() = default;
	virtual ~Out() = default;
	Out(const Out &) = delete;
	Out &operator=(const Out &) = delete;
	Out(Out &&) = default;
	Out &operator=(Out &&) = default;

	virtual void Add(Sock sock, const Data &data)
	{
		try {
			Lock lock(sync_);
			schedule_.emplace(std::make_pair(sock.GetFD(), data));
			Base::Add(std::move(sock));
		} catch (const SysEpoll::Error &) {
			throw;
		}
	}

	virtual void Remove(const Sock &sock)
	{
		try {
			schedule_.erase(sock.GetFD());
			Base::Remove(sock);
		} catch (const SysEpoll::Error &) {
			throw;
		}
	}

	virtual void Run() noexcept
	{
		try {
			debug("Running out scheduler");
			while (true) {
				Lock lock(sync_);
				if (schedule_.size() != 0) {
					auto events = SysEpoll::Wait(schedule_.size());
					debug("Out scheduler has " + std::to_string(events.size()) + " events");
					auto available_sockets = this->MatchEventsWithSockets(events);
					for (const auto &pair : available_sockets) {
						auto &socket = *pair.first;
						auto &event = *pair.second;
						ProcessEvent(socket, event, schedule_.at(socket.GetFD()));
					}
				}
			}
		} catch (SysEpoll::Error &polling_error) {
			debug(polling_error.what());
		} catch (const std::out_of_range &) {
			debug("Couldn't find schedule item!");
		} catch (const DataCorruption &e) {
			debug("Data corruption occured! Did not trim the already written data properly.");
			Remove(*e.sock);
		} catch (const typename Sock::WriteError &e) {
			debug("Write error on fd = " + std::to_string(e.fd));
			Remove(*e.ptr);
		} catch (const typename Sock::ConnectionClosedByPeer &e) {
			debug("Connection closed by peer on fd = " + std::to_string(e.fd));
			Remove(*e.ptr);
		}
	}

	virtual void ProcessEvent(const Sock &sock, const SysEpoll::Event &event, SchedItem<Data> &sched_item) noexcept
	{

		static constexpr auto flag_termination =
		    static_cast<const std::uint32_t>(SysEpoll::Description::Termination);
		static constexpr auto flag_error = static_cast<std::uint32_t>(SysEpoll::Description::Error);
		static constexpr auto flag_write = static_cast<std::uint32_t>(SysEpoll::Description::Write);

		if (event.description & flag_termination || event.description & flag_error) {
			Remove(sock);
			return;
		}
		try {
			if (event.description & flag_write) {
				auto &data = sched_item.data;
				auto written = sock.Write(data);
				if (written <= data.size()) {
					auto old_data_size = data.size();
					Data(data.begin() + written, data.end()).swap(data);

					if (old_data_size - written != data.size())
						throw DataCorruption{std::addressof(sock)};
				}
			}
		} catch (...) {
			throw;
		}
	}
};
}
};
#include <queue>

#endif // OUT_H
