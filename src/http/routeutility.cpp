#include "routeutility.h"
#include "parser.h"
#include <http/engine.h>
#include <algorithm>
#include <regex>

inline bool regex_match(const std::string &text, const std::string ex) {
    try {
        std::regex regex(ex);
        bool match = std::regex_match(text, regex);
        return match;
    } catch (const std::regex_error &) {
        return false;
    }
}

std::function<Http::Resolution(Http::Request)> RouteUtility::GetHandler(
    const Http::Request &request,
    const std::map<std::pair<Http::Method, std::string>, std::function<Http::Resolution(Http::Request)>> &routes) {
    auto strippedRoute = StripRoute(request.url);
    auto result =
        std::find_if(routes.begin(), routes.end(),
                     [&](const std::pair<std::pair<Http::Method, std::string>,
                                         std::function<Http::Resolution(Http::Request)>> &route) -> bool {
                         return (request.method == route.first.first) && regex_match(strippedRoute, route.first.second);
                     });

    if (result != routes.end()) {
        return result->second;
    }

    return nullptr;
}

std::string RouteUtility::StripRoute(const std::string &URI) {
    auto firstSlash = URI.find_first_of('/');
    return {URI.begin() + firstSlash, URI.end()};
}
