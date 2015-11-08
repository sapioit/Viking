//
// Created by vladimir on 09.08.2015.
//

#ifndef SOCKET_HTTPUTILITY_H
#define SOCKET_HTTPUTILITY_H

#include <io/socket/socket.h>
#include <http/request.h>

namespace Http
{
class Parser
{
	const IO::Socket &_sock;

	Request Init();
	Components::Method GetMethod(const std::string &str);
	Header GetHeader();

      public:
	Parser(const IO::Socket &sock) : _sock(sock) {}
	Request operator()();

	static Components::ContentType GetMimeTypeByExtension(const std::string &URI);
	static std::string StripRoute(const std::string &URI);
	static std::vector<std::string> Split(std::string source, char delimiter);
	// static std::string GetURI(const std::string& line);
};
};

#endif // SOCKET_HTTPUTILITY_H
