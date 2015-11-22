#include <server/server.h>
#include <http/response.h>
#include <http/request.h>
#include <json/json.h>
#include <iostream>

int main() {
    try {
        Web::Server server(1234);
        auto route1 = std::make_pair(std::make_pair(Http::Method::Get, "^\\/adsaf\\/json\\/(\\d+)$"),
                                     [](Http::Request req) -> Http::Response {
                                         // /adsaf/json/<int>

                                         Json::Value root(Json::arrayValue);
                                         Json::Value records(Json::arrayValue);
                                         Json::Value val;
                                         val["this"] = "that ";
                                         Json::Value a1{Json::arrayValue};
                                         Json::Value a2(Json::arrayValue);
                                         a1.append("1");
                                         a1.append("2");
                                         auto url_parts = req.SplitURL();
                                         a2.append(url_parts.at(2));
                                         a2.append("2");
                                         records.append(val);
                                         records.append(a1);
                                         records.append(a2);
                                         root.append(records);
                                         return {root};
                                     });

        auto route2 = std::make_pair(std::make_pair(Http::Method::Get, "^\\/adsaf\\/json\\/$"),
                                     [](Http::Request) -> Http::Response {
                                         // /adsaf/json/

                                         Json::Value root(Json::arrayValue);
                                         Json::Value records(Json::arrayValue);
                                         Json::Value a1(Json::arrayValue);
                                         Json::Value a2(Json::arrayValue);
                                         a1.append("1");
                                         a1.append("2");
                                         a2.append("3");
                                         a2.append("4");
                                         records.append(a1);
                                         records.append(a2);
                                         root.append(records);
                                         return {root};
                                     });
        Settings settings;
        settings.root_path = "/mnt/exthdd/server";
        settings.max_connections = 1000;
        settings.default_max_age = 500;
        server.SetSettings(settings);
        server.AddRoute(route1);
        server.AddRoute(route2);
        server.Run();
    } catch (std::exception &ex) {
        std::cerr << ex.what();
    }
    return 0;
}
