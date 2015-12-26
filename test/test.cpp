#include <server/server.h>
#include <http/resolution.h>
#include <http/request.h>
#include <json/json.h>
#include <iostream>
#include <iomanip>
#include <experimental/filesystem>

#ifdef __cpp_lib_experimental_filesystem

namespace fs = std::experimental::filesystem;

std::string trim_quotes(std::string str) {
  if (str.front() == '\"')
    str = str.substr(1, str.size());
  if (str.back() == '\"')
    str = str.substr(0, str.size());
  return str;
}

std::string url_encode(const std::string &value) {
  std::ostringstream escaped;
  escaped.fill('0');
  escaped << std::hex;

  for (std::string::const_iterator i = value.begin(), n = value.end(); i != n;
       ++i) {
    std::string::value_type c = (*i);

    // Keep alphanumeric and other accepted characters intact
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      escaped << c;
      continue;
    }

    // Any other characters are percent-encoded
    escaped << std::uppercase;
    escaped << '%' << std::setw(2) << int((unsigned char)c);
    escaped << std::nouppercase;
  }

  return escaped.str();
}

Http::Response list_directory(Http::Request req, const std::string &root_path) {
  try {
    fs::path root = root_path + req.url;
    std::ostringstream stream;
    stream << "<h1>Directory listing of " + req.url + "</h1>";
    for (auto it : fs::directory_iterator(root)) {
      fs::path p = it;
      stream << "<a href=\"";
      stream << url_encode(trim_quotes(p.filename())) << "\">";
      stream << trim_quotes(p.filename());
      stream << "</a><br/>";
    }
    Http::Response r{stream.str()};
    r.Set("Content-Type", "text/html; charset=utf-8");
    return r;
  } catch (...) {
    return {Http::StatusCode::NotFound};
  }
}
#endif
constexpr long fs_lib_v() {
#ifdef __cpp_lib_experimental_filesystem
  return __cpp_lib_experimental_filesystem;
#endif
  return 0;
}

int main() {
  try {
    Web::Server server(1234);
    Settings settings;
#ifdef __arm__
    settings.root_path = "/mnt/exthdd/server";
#else
    settings.root_path = "/mnt/hdd/store/basic";
#endif
    settings.max_connections = 1000;
    server.Initialize();
    server.AddRoute(Http::Method::Get, std::regex{"^\\/adsaf\\/json\\/(\\d+)$"},
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

    server.AddRoute(
        Http::Method::Get, std::regex{"^\\/adsaf\\/jsons\\/$"},
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

#ifdef __cpp_lib_experimental_filesystem

    server.AddRoute(Http::Method::Get, std::regex{"^([^.]+)$"},
                    [settings](Http::Request req) -> Http::Response {
                      // Matches any directory that doesn't have a dot in its
                      // name
                      if (req.url.back() != '/') {
                        Http::Response r{Http::StatusCode::Found};
                        r.Set("Location", req.url + '/');
                        return r;
                      }
                      return list_directory(req, settings.root_path);
                    });
#endif

    server.SetSettings(settings);
    server.Run();
  } catch (std::exception &ex) {
    std::cerr << ex.what();
  }
  return 0;
}
