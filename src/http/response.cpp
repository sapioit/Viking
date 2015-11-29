#include <http/response.h>
#include <http/request.h>
#include <misc/date.h>
#include <misc/dfa.h>

#include <sstream>
#include <utility>
#include <iomanip>
#include <iterator>

using namespace Http;

StatusCode Response::GetCode() const { return code_; }
void Response::SetCode(StatusCode code) { code_ = code; }

const Http::ContentType &Response::GetContentType() const { return _content_type; }
void Response::SetContentType(const Http::ContentType &value) { _content_type = value; }

std::size_t Response::ContentLength() const {
    if (GetType() == Type::File)
        return file_->size;
    if (GetType() == Type::Resource)
        return resource_.content().size();
    if (GetType() == Type::Text)
        return text_.size();
    return 0;
}

Response::Type Response::GetType() const { return type_; }

const Resource &Response::GetResource() const { return resource_; }

void Response::SetResource(const Resource &resource) {
    resource_ = resource;
    type_ = Type::Resource;
}

const std::string &Response::GetText() const { return text_; }

void Response::SetText(const std::string &text) {
    text_ = text;
    type_ = Type::Text;
}

CachePolicy Response::GetCachePolicy() const { return cache_policy_; }

void Response::SetCachePolicy(CachePolicy cache_policy) { cache_policy_ = cache_policy; }

bool Response::GetKeepAlive() const { return keep_alive_; }

void Response::SetKeepAlive(bool keep_alive) { keep_alive_ = keep_alive; }

Version Response::GetVersion() const { return version_; }

void Response::SetVersion(Version version) { version_ = version; }

void Response::Init() {
    version_ = {1, 1};
    keep_alive_ = false;
    type_ = Type::Text;
}

Response::Response() { Init(); }

Response::Response(const UnixFile *file) : code_(Http::StatusCode::OK), file_(file) {
    Init();
    type_ = Type::File;
}

Response::Response(StatusCode code) : code_(code) { Init(); }

Response::Response(const std::string &text) : code_(StatusCode::OK), text_({text.begin(), text.end()}) {
    Init();
    type_ = Type::Text;
}

Response::Response(const Resource &resource) : code_(StatusCode::OK), resource_(resource) {
    Init();
    type_ = Type::Resource;
}

Response::Response(const Json::Value &json)
    : code_(StatusCode::OK), text_(json.toStyledString()), _content_type(Http::ContentType::ApplicationJson) {
    Init();
    type_ = Type::Text;
}
