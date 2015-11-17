#include <server/server.h>
#include <server/dispatcher.h>
#include <io/schedulers/io_scheduler.h>
#include <http/parser.h>
#include <misc/storage.h>
#include <misc/debug.h>
#include <sys/types.h>
#include <unistd.h>

#include <utility>
#include <algorithm>
#include <iostream>
#include <thread>
#include <future>
#include <regex>
#include <map>
#include <functional>
#include <signal.h>

using namespace Web;

std::mutex Log::mLock;
std::string Log::_fn;

Server::Server(int port) : _port(port) {}

void Server::SetSettings(const Settings &s)
{
	Storage::setSettings(s);
	_maxPending = s.max_connections;
}
void Server::Run()
{
	signal(SIGPIPE, SIG_IGN);
	debug("Pid = " + std::to_string(getpid()));
	if (_port == -1)
		throw std::runtime_error("Port number not set!");
	try {
		auto master_socket = IO::Socket::start_socket(_port, _maxPending);
		debug("Master socket has fd = " + std::to_string(master_socket.GetFD()));

		IO::Scheduler watcher(std::move(master_socket), Dispatcher::Dispatch);
		while (true) {
			watcher.Run();
		}

	} catch (std::exception &ex) {
		Log::e(std::string("Server error: ").append(ex.what()));
		auto msg = std::string("Server error. Please see the log file. Last exception: ");
		msg += ex.what();
		msg += "\n";
		throw std::runtime_error(msg);
	}
}
int Server::GetMaxPending() const { return _maxPending; }

void Server::SetMaxPending(int maxPending) { _maxPending = maxPending; }
