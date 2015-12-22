#include <server/server.h>
#include <http/resolution.h>
#include <http/request.h>
#include <json/json.h>
#include <iostream>

int main() {
    try {
        Web::Server server(1234);
        auto route1 = std::make_pair(
                    std::make_pair(Http::Method::Get, "^\\/adsaf\\/json\\/(\\d+)$"),
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
        return {root.toStyledString()};
    });

    auto route2 = std::make_pair(
                std::make_pair(Http::Method::Get, "^\\/adsaf\\/json\\/$"),
                [](Http::Request) -> Http::Response {
            return {"This is a sample string response"};
});

    auto route3 = std::make_pair(
                std::make_pair(Http::Method::Get, "^\\/adsaf\\/jsons\\/$"),
                [](Http::Request) -> Http::Resolution {
            // /adsaf/jsons/
            auto future = std::async(std::launch::async, []() -> Http::Response {
                                         Json::Value root(Json::arrayValue);
                                         Json::Value records(Json::arrayValue);
                                         Json::Value a1(Json::arrayValue);
                                         Json::Value a2(Json::arrayValue);
                                         a1.append("1");
                                         a1.append("2");
                                         a2.append("4dn");
                                         a2.append("7");
                                         records.append(a1);
                                         records.append(a2);
                                         root.append(records);
                                         std::this_thread::sleep_for(std::chrono::seconds(5));
                                         return {root.toStyledString()};
                                     });
            return {std::move(future)};
});
    Settings settings;
#ifdef __arm__
    settings.root_path = "/mnt/exthdd/server";
#else
    settings.root_path = "/mnt/hdd/store/basic";
#endif
    settings.max_connections = 1000;
    server.SetSettings(settings);
    server.AddRoute(route1);
    server.AddRoute(route3);
    server.AddRoute(route2);
    server.Run();
} catch (std::exception &ex) {
    std::cerr << ex.what();
}
return 0;
}
