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
#ifndef RESPONSE_H
#define RESPONSE_H

#include <http/header.h>
#include <http/version.h>
#include <misc/resource.h>
#include <io/buffers/unix_file.h>

#include <string>
#include <future>

namespace http {
using namespace http;
class Response {
    public:
    enum class Type { Resource, File, Text };
    Response();
    Response(StatusCode);
    Response(const std::string &);
    Response(http::StatusCode, const std::string &);
    Response(const resource &);
    Response(resource &&);
    virtual ~Response() = default;

    Version GetVersion() const;
    void SetVersion(Version);

    StatusCode GetCode() const;
    void SetCode(StatusCode GetCode);

    std::size_t ContentLength() const;

    Type GetType() const;
    void SetType(Type type) noexcept;

    const resource &GetResource() const;
    void SetResource(const resource &text);

    const std::string &GetText() const;
    void SetText(const std::string &);

    const io::unix_file *GetFile() const;
    void SetFile(io::unix_file *file) noexcept;

    bool GetKeepAlive() const noexcept;
    void Set(const std::string &field, const std::string &value) noexcept;
    const std::vector<std::pair<std::string, std::string>> &GetFields() const noexcept;

    private:
    std::vector<std::pair<std::string, std::string>> fields_;
    Version version_ = {1, 1};
    StatusCode code_;
    Type type_;
    resource resource_;
    std::string text_;
    const io::unix_file *file_ = nullptr;
    void Init();
};
};

#endif // RESPONSE_H
