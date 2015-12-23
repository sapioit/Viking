#include <server/server.h>
#include <http/resolution.h>
#include <http/request.h>
#include <json/json.h>
#include <iostream>

int main() {
    try {
        Web::Server server(1234);
        server.AddRoute(Http::Method::Get, "^\\/adsaf\\/json\\/(\\d+)$",
                        [](auto req) -> Http::Response {
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

        server.AddRoute(Http::Method::Get, "^\\/adsaf\\/json\\/$",
                        [](auto) -> Http::Response {
            return {"This is a sample string response"};
        });

        server.AddRoute(Http::Method::Get, "^\\/adsaf\\/jsons\\/$",
                        [](auto) -> Http::Resolution {
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
        server.Run();
    } catch (std::exception &ex) {
        std::cerr << ex.what();
    }
    return 0;
}
