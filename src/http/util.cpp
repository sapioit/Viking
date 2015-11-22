#include <http/util.h>
#include <regex>
using namespace Http;

bool Util::IsPassable(const Http::Request &request) noexcept {
    switch (request.method) {
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
    return true;
}

bool Util::ExtensionAllowed(const std::string &url) noexcept {
    static std::regex extensions(".*\\.(jpg|jpeg|png|gif|zip|pdf|mp4|html|json|mkv)$",
                                 std::regex::ECMAScript | std::regex::icase);
    return std::regex_match(url, extensions);
}

Http::ContentType Util::GetMimeType(const std::string &url) noexcept {

    auto dot = url.find_last_of('.');
    std::string ext(url.begin() + dot + 1, url.end());

    if (ext == "png")
        return ContentType::ImagePng;
    if (ext == "jpg")
        return ContentType::ImageJpeg;
    if (ext == "mp4")
        return ContentType::MovieMp4;
    if (ext == "html" || ext == "htm")
        return ContentType::TextHtml;
    if (ext == "json")
        return ContentType::ApplicationJson;

    return ContentType::TextPlain;
}
