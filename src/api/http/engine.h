//
// Created by vladimir on 09.08.2015.
//

#ifndef SOCKET_HTTPUTILITY_H
#define SOCKET_HTTPUTILITY_H

#include <io/socket/socket.h>
#include <http/request.h>
#include <http/parser.h>
#include <iostream>
#include <string>

namespace Http
{
class Engine
{
	const IO::Socket &socket_;
	http_parser_settings settings_;
	http_parser parser_;
	Request request_;

	std::string header_field;

	inline static Engine *GetMe(http_parser *parser);

	void AssignMethod(http_method method_numeric);

	public:
	Engine(const IO::Socket &socket);

	Request operator()();

	static Components::ContentType GetMimeTypeByExtension(const std::string &URI);
	static std::string StripRoute(const std::string &URI);
	static std::vector<std::string> Split(std::string source, char delimiter);
	//	// static std::string GetURI(const std::string& line);
	Components::ContentType GetMimeType(const std::string &);
	std::vector<Components::ContentType> GetAcceptedEncodings(const std::string &);
};
};

#endif // SOCKET_HTTPUTILITY_H
