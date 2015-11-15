#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <set>
#include <memory>
#include <algorithm>
#include <misc/debug.h>

namespace IO
{
class FileWatcher
{
      protected:
    std::set<Socket> watched_files_;

      public:
	struct FileNotFound {
		int fd;
	};
	FileWatcher() = default;
	virtual ~FileWatcher() = default;

    void Add(Socket file) noexcept { watched_files_.emplace(std::move(file)); }

    void Remove(const Socket &file)
	{
		auto elements_removed = watched_files_.erase(file);
		if (elements_removed == 0) {
			throw FileNotFound{file.GetFD()};
		}
	}
};
}

#endif
