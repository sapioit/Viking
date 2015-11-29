#include <http/resolution.h>

using namespace Http;
Resolution::Resolution(const Http::Response &response) : response(response), type(Type::Sync) {}

Resolution::Resolution(std::future<Response> future) : future(std::move(future)), type(Type::Async) {}

Resolution::Type Resolution::GetType() const noexcept { return type; }

std::future<Response> &Resolution::GetFuture() noexcept { return future; }

Response &Resolution::GetResponse() noexcept { return response; }
