//
// Created by vladimir on 08.08.2015.
//

#include <http/engine.h>
#include <http/request.h>
#include <regex>

using namespace Http;

const std::vector<Http::ContentType> Request::Accepts() const {
    // TODO
    return {};
}

std::vector<std::string> Request::SplitURL() const { return Http::Engine::Split(url, '/'); }

bool Http::Request::IsPassable() const {
    switch (method) {
    case Http::Method::Get:
        return true;
    case Http::Method::Post:
        return true;
    case Http::Method::Put:
        return true;
    case Http::Method::Delete:
        return true;
    case Http::Method::Head:
        return true;
    default:
        return false;
    }
}

bool Http::Request::IsResource() const {
    std::regex extensions(".*\\.(jpg|jpeg|png|gif|zip|pdf|mp4|html|json)$", std::regex::ECMAScript | std::regex::icase);
    bool match = std::regex_match(url, extensions);
    return match;
}
