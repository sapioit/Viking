#include <http/response.h>
#include <http/request.h>
#include <misc/date.h>
#include <misc/log.h>
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
    if (GetBodyType() == BodyType::File)
        return file_->size;
    if (GetBodyType() == BodyType::Resource)
        return resource_.content().size();
    if (GetBodyType() == BodyType::Text)
        return text_.size();
    return 0;
}

Response::BodyType Response::GetBodyType() const { return body_type_; }

const Resource &Response::GetResource() const { return resource_; }

void Response::SetResource(const Resource &resource) {
    resource_ = resource;
    body_type_ = BodyType::Resource;
}

const std::string &Response::GetText() const { return text_; }

void Response::SetText(const std::string &text) {
    text_ = text;
    body_type_ = BodyType::Text;
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
    body_type_ = BodyType::Text;
}

Response::Response() { Init(); }

Response::Response(const UnixFile *file) : code_(Http::StatusCode::OK), file_(file) {
    Init();
    body_type_ = BodyType::File;
}

Response::Response(StatusCode code) : code_(code) { Init(); }

Response::Response(const std::string &text) : code_(StatusCode::OK), resource_({text.begin(), text.end()}) {
    Init();
    body_type_ = BodyType::Text;
}

Response::Response(const Resource &resource) : code_(StatusCode::OK), resource_(resource) {
    Init();
    body_type_ = BodyType::Resource;
}

Response::Response(const Json::Value &json)
    : code_(StatusCode::OK), text_(json.toStyledString()), _content_type(Http::ContentType::ApplicationJson) {
    Init();
    body_type_ = BodyType::Text;
}
