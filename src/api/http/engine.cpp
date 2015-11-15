#ifndef ENGINE_H
#define ENGINE_H

#include <http/engine.h>
#include <http/components.h>
#include <misc/debug.h>
#include <sstream>

Http::Engine *Http::Engine::GetMe(http_parser *parser) { return reinterpret_cast<Engine *>(parser->data); }

void Http::Engine::AssignMethod(http_method method_numeric)
{
	auto *method_str = http_method_str(method_numeric);
	auto method = Http::Components::MethodMap.find(method_str);
	if (method != Http::Components::MethodMap.end())
		request_.method = method->second;
}

Http::Engine::Engine(const IO::Socket &socket) : socket_(socket)
{
	settings_.on_message_begin = [](http_parser *) -> int { return 0; };
	settings_.on_message_complete = [](http_parser *) -> int {
		return 0;

	};
	settings_.on_headers_complete = [](http_parser *parser) -> int {
		auto *me = GetMe(parser);
		me->request_.version.v_major = parser->http_major;
		me->request_.version.v_minor = parser->http_minor;
		me->AssignMethod(static_cast<http_method>(parser->method));
		return 0;
	};
	settings_.on_url = [](http_parser *parser, const char *at, size_t length) -> int {
		auto *me = GetMe(parser);
		me->request_.URI = {at, at + length};
		return 0;

	};
	settings_.on_header_field = [](http_parser *parser, const char *at, size_t length) -> int {
		auto *me = GetMe(parser);
		me->header_field = std::string{at, at + length};
		debug(me->header_field);

		return 0;
	};
	settings_.on_header_value = [](http_parser *parser, const char *at, size_t length) -> int {
		std::string value(at, at + length);
		auto *me = GetMe(parser);
		me->request_.header.fields.insert(std::make_pair(me->header_field, value));

		debug(value);
		return 0;
	};
	settings_.on_body = [](http_parser *, const char *at, size_t length) -> int {
		std::string body(at, at + length);
		debug(body);

		return 0;

	};
	parser_.data = reinterpret_cast<void *>(this);
	http_parser_init(&parser_, HTTP_REQUEST);
}

Http::Request Http::Engine::operator()()
{
	try {
		auto data = socket_.ReadSome<std::string>();
		http_parser_execute(&parser_, &settings_, &data.front(), data.size());
		return request_;
		/* TODO in the future, take the state into account, because we
		     * might not be getting the full header
		     */
	} catch (...) {
		throw;
	}
	return {};
}

Http::Components::ContentType Http::Engine::GetMimeTypeByExtension(const std::string &URI)
{
	auto dot = URI.find_last_of('.');
	std::string ext(URI.begin() + dot + 1, URI.end());

	using namespace Components;

	if (ext == "png")
		return ContentType::ImagePng;
	if (ext == "jpg")
		return ContentType::ImageJpeg;
	if (ext == "mp4")
		return ContentType::MovieMp4;
	if (ext == "html" || ext == "htm")
		return ContentType::TextHtml;
	if (ext == "json")
		return ContentType::ApplicationJson;

	return ContentType::TextPlain;
}

std::string Http::Engine::StripRoute(const std::string &URI)
{
	auto firstSlash = URI.find_first_of('/');
	return {URI.begin() + firstSlash, URI.end()};
}

std::vector<std::string> Http::Engine::Split(std::string source, char delimiter)
{
	std::vector<std::string> result;
	std::istringstream ss(source); // Turn the string into a stream.
	std::string tok;

	while (std::getline(ss, tok, delimiter)) {
		if (!tok.empty())
			result.push_back(tok);
	}
	return result;
}
std::vector<Http::Components::ContentType> Http::Engine::GetAcceptedEncodings(const std::string &)
{
	return std::vector<Components::ContentType>();
}

Http::Components::ContentType Http::Engine::GetMimeType(const std::string &)
{
	// TODO parse line and get it
	return Components::ContentType::ApplicationJson;
}

#endif
