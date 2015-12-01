#include <io/schedulers/file_container.h>

// void IO::SocketContainer::Add(IO::Socket file) noexcept {
// watched_files_.emplace_back(std::move(file)); }

// void IO::SocketContainer::Remove(const IO::Socket &file) noexcept
//{
//	for (auto index = 0u; index < watched_files_.size(); ++index) {
//		if (watched_files_[index].GetFD() == file.GetFD()) {
//			// auto fd = watched_files_[index].GetFD();
//			watched_files_.erase(watched_files_.begin() + index);
//			debug("Removed socket with fd = " + std::to_string(fd) +
//"
// from the socket watcher");
//			return;
//		}
//    }
//}

// const std::vector<IO::Socket> &IO::SocketContainer::Get() const noexcept
//{
// return watched_files_;
//}
