#ifndef RESPONSE_H
#define RESPONSE_H

#include <http/header.h>
#include <http/request.h>
#include <http/cache_policy.h>
#include <json/json.h>
#include <misc/resource.h>
#include <io/buffers/unix_file.h>

#include <string>
#include <future>

namespace Http {
using namespace Http;
class Response {
    void Init();

    public:
    enum class Type { Resource, File, Text };
    Response() = default;
    Response(StatusCode);
    Response(const std::string &);
    Response(Http::StatusCode, const std::string &);
    Response(const Resource &);
    virtual ~Response() = default;

    CachePolicy GetCachePolicy() const;
    void SetCachePolicy(CachePolicy);

    bool GetKeepAlive() const;
    void SetKeepAlive(bool);

    Version GetVersion() const;
    void SetVersion(Version);

    StatusCode GetCode() const;
    void SetCode(StatusCode GetCode);

    const Http::ContentType &GetContentType() const;
    void SetContentType(const Http::ContentType &value);

    std::size_t ContentLength() const;

    Type GetType() const;
    void SetType(Type type) noexcept;

    const Resource &GetResource() const;
    void SetResource(const Resource &text);

    const std::string &GetText() const;
    void SetText(const std::string &);

    const UnixFile *GetFile() const;
    void SetFile(UnixFile *file) noexcept;

    private:
    Version version_ = {1, 1};
    StatusCode code_ = Http::StatusCode::OK;
    Type type_;

    Resource resource_;
    std::string text_;
    const UnixFile *file_ = nullptr;

    Http::ContentType _content_type = Http::ContentType::TextPlain;
    bool keep_alive_;
    CachePolicy cache_policy_;
};
};

#endif // RESPONSE_H
