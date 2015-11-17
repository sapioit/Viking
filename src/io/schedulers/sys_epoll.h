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

	epoll_event *FindEvent(int);

	public:
	struct Event {
		public:
		int file_descriptor;
		std::uint32_t description;
		Event() noexcept = default;
		Event(int fd, int description) noexcept;
		bool operator<(const Event &other) const { return file_descriptor < other.file_descriptor; }
	};

	struct Error : public std::runtime_error {
		Error(const std::string &err);
	};

	enum class Description { Read = EPOLLIN, Write = EPOLLOUT, Error = EPOLLERR, Termination = EPOLLRDHUP };
	void Schedule(int, std::uint32_t);
	void Modify(int, std::uint32_t);
	void Remove(int);
	std::vector<Event> Wait(std::uint32_t = 1000) const;

	SysEpoll();
	virtual ~SysEpoll() = default;
};

#endif
