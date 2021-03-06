/*
Copyright (C) 2015 Voinea Constantin Vladimir

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/
#include <experimental/filesystem>
#include <http/request.h>
#include <http/resolution.h>
#include <iomanip>
#include <iostream>
#include <json/json.h>
#include <server/server.h>
namespace fs = std::experimental::filesystem;

int main() {
  web::server server(1234);
  configuration settings;
  settings.root_path = "/home/vladimir";
  settings.max_connections = 1000;
  settings.default_max_age = 99999999;
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
                     return {req, root.toStyledString()};
                   });
  server.add_route(http::method::Get, std::regex{"^\\/adsaf\\/jsons\\/$"},
                   [](auto req) -> http::resolution {
                     auto future = std::async(
                         std::launch::async,
                         [](auto req) -> http::response {
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
                           return {req, root.toStyledString()};
                         },
                         req);
                     return {std::move(future)};
                   });

  server.set_config(settings);
  server.run();
  return 0;
}
