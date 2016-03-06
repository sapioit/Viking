#include <server/server.h>
#include <http/resolution.h>
#include <http/request.h>
#include <json/json.h>
#include <iostream>
#include <iomanip>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

int main() {
  web::server server(1234);
  configuration settings;
  settings.root_path = "/home/vladimir";
  settings.max_connections = 1000;
  try {
    server.init();
  } catch (const web::server::port_in_use &e) {
    std::cerr << "Port " << e.port << " is in use!" << std::endl;
    std::exit(1);
  }

  server.add_route(http::method::Get, std::regex{"^\\/adsaf\\/json\\/(\\d+)$"},
                   [](auto req) -> http::response {
                     Json::Value root(Json::arrayValue);
                     Json::Value records(Json::arrayValue);
                     Json::Value val;
                     val["this"] = "that ";
                     Json::Value a1{Json::arrayValue};
                     Json::Value a2(Json::arrayValue);
                     a1.append("1");
                     a1.append("2");
                     auto url_parts = req.split_url();
                     a2.append(url_parts.at(2));
                     a2.append("2");
                     records.append(val);
                     records.append(a1);
                     records.append(a2);
                     root.append(records);
                     return {root.toStyledString()};
                   });

  server.add_route(http::method::Get, std::regex{"^\\/adsaf\\/jsons\\/$"},
                   [](auto) -> http::resolution {
                     auto future =
                         std::async(std::launch::async, []() -> http::response {
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

  server.set_config(settings);
  server.run();
  return 0;
}
