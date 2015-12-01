#ifndef ROUTESMANAGER_H
#define ROUTESMANAGER_H

#include <http/request.h>
#include <http/response.h>
#include <http/resolution.h>
#include <map>
#include <functional>

class RouteUtility {
    public:
    static std::function<Http::Resolution(Http::Request)> GetHandler(
        const Http::Request &request,
        const std::map<std::pair<Http::Method, std::string>, std::function<Http::Resolution(Http::Request)>> &routes);

    static std::string StripRoute(const std::string &URI);
};

#endif // ROUTESMANAGER_H
