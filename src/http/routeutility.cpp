#include "routeutility.h"
#include "parser.h"
#include <http/engine.h>
#include <algorithm>
#include <regex>

std::function<Http::Resolution(Http::Request)> RouteUtility::GetHandler(const Http::Request &request,
                                                                        const RouteMap &routes) {
    auto strippedRoute = StripRoute(request.url);
    auto result = std::find_if(routes.begin(), routes.end(), [&](const auto &route) -> bool {
        return request.method == route.first.first && route.first.second(strippedRoute);
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
