#include <io/schedulers/sys_epoll.h>
#include <misc/debug.h>
#include <cstring>
#include <algorithm>
#include <errno.h>
#include <cassert>

SysEpoll::SysEpoll()
{
	efd_ = epoll_create1(0);
	if (efd_ == -1)
		throw Error("Could not start polling");
	debug("SysEpoll instance with fd = " + std::to_string(efd_));
}

void SysEpoll::Schedule(int file_descriptor, std::uint32_t flags)
{
	struct epoll_event ev;
	memset(&ev, 0, sizeof(struct epoll_event));
	ev.data.fd = file_descriptor;
	ev.events = flags | EPOLLET;
	if (-1 == epoll_ctl(efd_, EPOLL_CTL_ADD, ev.data.fd, &ev)) {
		if (errno != EEXIST) {
			throw Error("Could not add the file with fd = " + std::to_string(file_descriptor) +
				    " to the polling queue");
		} else {
			Modify(file_descriptor, flags);
		}
	} else {
		debug("Scheduled file descriptor " + std::to_string(file_descriptor));
	}
	events_.push_back(ev);
}

void SysEpoll::Modify(int file_descriptor, std::uint32_t flags)
{
	auto *event = FindEvent(file_descriptor);
	if (event) {
		event->events |= flags;
		if (-1 == epoll_ctl(efd_, EPOLL_CTL_MOD, event->data.fd, event)) {
			debug("Could not modify the event with fd = " + std::to_string(file_descriptor) + " errno = " +
			      std::to_string(errno));
		}
	}
}

void SysEpoll::Remove(int file_descriptor)
{
	// FIXME
	epoll_event *event = nullptr;
	auto event_it = std::find_if(events_.begin(), events_.end(), [file_descriptor, &event](epoll_event &ev) {
		if (file_descriptor == ev.data.fd) {
			event = &ev;
			return true;
		}
		return false;
	});
	if (event_it != events_.end()) {
		auto *event = std::addressof(*event_it);
		if (-1 == epoll_ctl(efd_, EPOLL_CTL_DEL, file_descriptor, event))
			throw Error("Could not remove the file with fd = " + std::to_string(file_descriptor) +
				    " from the OS queue");
		debug("Removed file descriptor " + std::to_string(file_descriptor) + " from SysEpoll");
		events_.erase(event_it);
	} else {
		debug("SysEpoll could not find event with fd = " + std::to_string(file_descriptor));
	}
}

static std::vector<SysEpoll::Event> CreateEvents(const std::vector<epoll_event> &events) noexcept
{
	std::vector<SysEpoll::Event> epoll_events;
	for (const auto &event : events) {
		epoll_events.emplace_back(event.data.fd, event.events);
		debug("Event with fd = " + std::to_string(event.data.fd) + " was reported to be active");
	}
	return epoll_events;
}
std::vector<SysEpoll::Event> SysEpoll::Wait(std::uint32_t chunk_size) const
{
	std::vector<epoll_event> active_files;
	active_files.resize(chunk_size);

	auto events_number = epoll_wait(efd_, &active_files.front(), chunk_size, -1);

	if (-1 == events_number) {
		active_files.resize(0);
		if (errno != EINTR)
			throw Error("Could not poll for events. errno = " + std::to_string(errno));
	} else {
		debug("Waited for " + std::to_string(events_number) + " events");
		active_files.resize(events_number);
	}

	return CreateEvents(active_files);
}

epoll_event *SysEpoll::FindEvent(int file_descriptor)
{
	auto event_it = std::find_if(events_.begin(), events_.end(), [file_descriptor](const epoll_event &ev) {
		return (file_descriptor == ev.data.fd);
	});
	return (event_it == events_.end() ? nullptr : std::addressof(*event_it));
}
SysEpoll::Event::Event(int fd, int description) noexcept : file_descriptor(fd), description(description) {}

SysEpoll::Error::Error(const std::string &err) : std::runtime_error(err) {}
