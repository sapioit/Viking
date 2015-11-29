#include <http/resolution.h>

using namespace Http;
Resolution::Resolution(const Http::Response& response) : response(response), type(Type::Sync)
{

}

Resolution::Resolution(std::future<Response> future) : future(std::move(future)), type(Type::Async)
{

}

