#ifndef SERVER_H
#define SERVER_H

#include <io/socket/socket.h>
#include <server/dispatcher.h>
#include <misc/settings.h>
#include <misc/log.h>

#include <vector>
#include <memory>
#include <fstream>
namespace Web
{
class Server
{
	public:
	Server(int);
	template <class T> void AddRoute(T route) { dispatcher_.AddRoute(route); }
	void Run();
	void SetSettings(const Settings &);

	int GetMaxPending() const;
	void SetMaxPending(int);

	private:
	Dispatcher dispatcher_;
	int _port = -1;
	int _maxPending;
};
};

#endif // SERVER_H
