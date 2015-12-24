#ifndef RESPONSE_H
#define RESPONSE_H

#include <http/header.h>
#include <http/version.h>
#include <misc/resource.h>
#include <io/buffers/unix_file.h>

#include <string>
#include <future>

namespace Http {
using namespace Http;
class Response {
    public:
    enum class Type { Resource, File, Text };
    Response();
    Response(StatusCode);
    Response(const std::string &);
    Response(Http::StatusCode, const std::string &);
    Response(const Resource &);
    virtual ~Response() = default;

    Version GetVersion() const;
    void SetVersion(Version);

    StatusCode GetCode() const;
    void SetCode(StatusCode GetCode);

    std::size_t ContentLength() const;

    Type GetType() const;
    void SetType(Type type) noexcept;

    const Resource &GetResource() const;
    void SetResource(const Resource &text);

    const std::string &GetText() const;
    void SetText(const std::string &);

    const UnixFile *GetFile() const;
    void SetFile(UnixFile *file) noexcept;

    bool GetKeepAlive() const noexcept;
    void Set(const std::string &field, const std::string &value) noexcept;
    const std::vector<std::pair<std::string, std::string>> &GetFields() const noexcept;

    private:
    std::vector<std::pair<std::string, std::string>> fields_;
    Version version_ = {1, 1};
    StatusCode code_;
    Type type_;
    Resource resource_;
    std::string text_;
    const UnixFile *file_ = nullptr;
    void Init();
};
};

#endif // RESPONSE_H
