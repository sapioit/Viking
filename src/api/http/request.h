//
// Created by vladimir on 08.08.2015.
//

#ifndef SOCKET_REQUEST_H
#define SOCKET_REQUEST_H

#include <http/components.h>
#include <http/header.h>
#include <string>

namespace Http
{

class Request
{
	public:
	struct Version {
		unsigned short major, minor;
	};

	Http::Method method;
	Version version;
	Header header;
	std::string url, body;

	Request() = default;
	virtual ~Request() = default;

	bool IsPassable() const;
	bool IsResource() const;

	const std::vector<Http::ContentType> Accepts() const;

	std::vector<std::string> SplitURL() const;
};
}

#endif // SOCKET_REQUEST_H
