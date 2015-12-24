#include <http/util.h>
#include <regex>
using namespace Http;

bool Util::IsPassable(const Http::Request &request) noexcept {
    switch (request.method) {
    case Http::Method::Get:
        return true;
    case Http::Method::Post:
        return true;
    case Http::Method::Put:
        return true;
    case Http::Method::Delete:
        return true;
    case Http::Method::Head:
        return true;
    default:
        return false;
    }
    return true;
}

bool Util::ExtensionAllowed(const std::string &url) noexcept {
    static std::regex extensions(".*\\.(jpg|jpeg|png|gif|zip|pdf|mp4|html|json|mkv|js)$",
                                 std::regex::ECMAScript | std::regex::icase);
    return std::regex_match(url, extensions);
}
bool Util::IsComplete(const Request &request) noexcept {
    if (CanHaveBody(request.method)) {
        auto cl_it = request.header.fields.find(Http::Header::Fields::Content_Length);
        if (cl_it != request.header.fields.end()) {
            auto content_length = static_cast<std::size_t>(std::atoi(cl_it->second.c_str()));
            if (request.body.size() < content_length)
                return false;
        }
        return true;
    }
    return true;
}

bool Util::CanHaveBody(Method method) noexcept {
    switch (method) {
    case Http::Method::Put:
        return true;
    case Http::Method::Post:
        return true;
    case Http::Method::Options:
        return true;
    default:
        return false;
    }
}
