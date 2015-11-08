#ifndef SYS_EPOLL_H
#define SYS_EPOLL_H

#include <sys/epoll.h>
#include <set>
#include <vector>
#include <stdexcept>

class SysEpoll
{
	int efd_;
	std::vector<epoll_event> events_;

      public:
	struct Event {
	      public:
		int file_descriptor;
		std::uint32_t description;
		Event() noexcept = default;
		Event(int fd, int description) noexcept;
		bool operator<(const Event &other) const { return file_descriptor < other.file_descriptor; }
	};
	template <typename Sock> struct EventComparer {
		bool operator()(const Sock &p_left, const SysEpoll::Event &p_right)
		{
			return p_left.GetFD() < p_right.file_descriptor;
		}

		bool operator()(const SysEpoll::Event &p_left, const Sock &p_right)
		{
			return p_left.file_descriptor < p_right.GetFD();
		}
	};

	struct Error : public std::runtime_error {
		Error(const std::string &err);
	};
	enum class Description { Read = EPOLLIN, Write = EPOLLOUT, Error = EPOLLERR, Termination = EPOLLRDHUP };
	void Schedule(int, std::uint32_t);
	void Remove(int);
	std::set<Event> Wait(std::uint32_t = 1000) const;
	virtual std::uint32_t GetBasicFlags() const noexcept = 0;

	SysEpoll();
	virtual ~SysEpoll() = default;
};

#endif
