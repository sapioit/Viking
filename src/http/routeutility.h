#ifndef ROUTESMANAGER_H
#define ROUTESMANAGER_H

#include <http/request.h>
#include <http/response.h>
#include <http/resolution.h>
#include <map>
#include <functional>

typedef std::map<std::pair<Http::Method, std::string>, std::function<Http::Resolution(Http::Request)>> RouteMap;
class RouteUtility {
    public:
    static std::function<Http::Resolution(Http::Request)> GetHandler(const Http::Request &request,
                                                                   const RouteMap &routes);
};

#endif // ROUTESMANAGER_H
