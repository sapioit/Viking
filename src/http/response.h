#ifndef RESPONSE_H
#define RESPONSE_H

#include <http/header.h>
#include <http/request.h>
#include <json/json.h>
#include <misc/resource.h>
#include <io/buffers/unix_file.h>

#include <string>
#include <iostream>

namespace Http
{
using namespace Http;
class Response
{
	public:
	Response();
	Response(const Request &, const UnixFile *);
	Response(const Request &, int);
	Response(const Request &, StatusCode);
	Response(const Request &, const std::string &);
	Response(const Request &, int, const std::string &);
	Response(const Request &, Http::StatusCode, const std::string &);
	Response(const Request &, const Resource &);
	Response(const Request &, const Json::Value &);
	Response(const Response &) = default;
	Response(Response &&) = default;
	Response &operator=(const Response &) = default;
	~Response() = default;

	bool should_cache() const;
	std::uint32_t get_expiry() const;
	bool should_close() const;
	bool has_body() const;
	bool has_resource() const;
	bool is_error() const;

	int GetCode() const;
	void SetCode(int GetCode);

	const Http::ContentType &GetContentType() const;
	void SetContentType(const Http::ContentType &value);

	std::size_t ContentLength() const;
	const Resource &getResource() const;
	void setResource(const Resource &resource);

	std::string end_str() const;
	std::string header_str() const;
	std::string str() const;
	const Request &getRequest() const;

	std::string getText() const;
	void setText(const std::string &text);

	private:
	Request _request;
	Resource _resource;
	int _code;
	std::string _text;
	const UnixFile *file_ = nullptr;
	Http::ContentType _content_type = Http::ContentType::TextPlain;
};
};

#endif // RESPONSE_H
