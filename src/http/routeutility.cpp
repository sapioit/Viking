#include "routeutility.h"
#include "parser.h"
#include <http/engine.h>
#include <algorithm>
#include <regex>

std::function<Http::Resolution(Http::Request)> RouteUtility::GetHandler(const Http::Request &request,
                                                                        const RouteMap &routes) {
    auto strippedRoute = Http::Engine::StripRoute(request.url);
    auto result = std::find_if(routes.begin(), routes.end(),
                               [&](const std::pair<std::pair<Http::Method, std::string>,
                                                   std::function<Http::Resolution(Http::Request)>> &route) -> bool {
                                   auto &method = route.first.first;
                                   if (request.method != method)
                                       return false;
                                   auto &pattern = route.first.second;
                                   std::regex regex(pattern);
                                   return std::regex_match(strippedRoute, regex);
                               });

    if (result != routes.end()) {
        return result->second;
    }

    return nullptr;
}
