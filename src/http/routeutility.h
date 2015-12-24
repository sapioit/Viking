#ifndef ROUTESMANAGER_H
#define ROUTESMANAGER_H

#include <http/request.h>
#include <http/response.h>
#include <http/resolution.h>
#include <map>
#include <functional>

class RouteUtility {
    public:
    typedef std::function<bool(std::string)> RouteValidator;
    typedef std::pair<Http::Method, RouteValidator> MethodRouteValidatorPair;
    typedef std::function<Http::Resolution(Http::Request)> HttpHandler;
    typedef std::pair<MethodRouteValidatorPair, HttpHandler> Route;

    typedef std::vector<Route> RouteMap;

    static HttpHandler GetHandler(const Http::Request &request, const RouteMap &routes);

    static std::string StripRoute(const std::string &URI);
};

#endif // ROUTESMANAGER_H
