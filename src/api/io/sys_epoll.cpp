#include <io/sys_epoll.h>

using namespace SysEpoll;

SysEpoll::SysEpoll() {
  efd_ = epoll_create1(0);
  if (efd == -1)
    throw Error("Could not start polling");
}

void SysEpoll::Schedule(int file_descriptor, std::uint32_t flags) {
  struct epoll_event ev;
  memset(&ev, 0, sizeof(struct epoll_event));
  ev.data.fd = file_descriptor;
  ev.events = flags | EPOLLET;
  if (-1 == epoll_ctl(_efd, EPOLL_CTL_ADD, ev.data.fd, &ev))
    throw Error("Could not add the file with fd = " +
                std::to_string(file_descriptor) + " to the polling queue");
}

void SysEpoll::Remove(int file_descriptor) {

  epoll_event *event = nullptr;
  std::remove_if(events_.begin(), events_.end(),
                 [file_descriptor](const epoll_event &ev) {
                   if (file_descriptor == ev.data.fd) {
                     event = &ev;
                     return true;
                   }
                   return false;
                 });

  if (event != nullptr)
    if (-1 == epoll_ctl(efd_, EPOLL_CTL_DEL, file_descriptor, event))
      throw Error("Could not remove the file with fd = " +
                  std::to_string(file_descriptor) + " from the OS queue");
}

static auto CreateEvents(const std::vector<epoll_event &> events) noexcept {
  std::vector<Event> epoll_events;
  for (const auto &active_file : active_files) {
    epoll_events.emplace_back(active_file.data.fd, active_file.events);
  }
  return epoll_events;
}

auto SysEpoll::Wait(std::uint32_t chunk_size) const {
  std::vector<epoll_event> active_files;
  active_files.reserve(chunk_size);

  auto events_number = epoll_pwait(efd_, &active_files.front(), chunk_size, 0);

  if (-1 == events_number)
    throw Error("Could not poll for events");

  active_files.resize(events_number);

  return CreateEvents(active_files);
}