#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <set>
#include <io/socket/socket.h>
#include <memory>
#include <algorithm>
#include <misc/debug.h>

namespace IO
{
class SocketContainer
{
	protected:
	std::vector<Socket> watched_files_;

	public:
	struct FileNotFound {
		int fd;
	};
	SocketContainer() = default;
	virtual ~SocketContainer() = default;

    void Add(Socket file) noexcept;

    void Remove(const Socket &file) noexcept;
};
}

#endif
