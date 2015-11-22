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
    static std::regex extensions(".*\\.(jpg|jpeg|png|gif|zip|pdf|mp4|html|json|mkv)$",
                                 std::regex::ECMAScript | std::regex::icase);
    return std::regex_match(url, extensions);
}
