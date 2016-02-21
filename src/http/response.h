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
class response {
    public:
    enum class type { Resource, File, Text };
    response();
    response(status_code);
    response(const std::string &);
    response(http::status_code, const std::string &);
    response(const resource &);
    response(resource &&);
    virtual ~response() = default;

    version get_version() const;
    void set_version(version);

    status_code get_code() const;
    void set_code(status_code get_code);

    std::size_t content_len() const;

    type get_type() const;
    void set_type(type type) noexcept;

    const resource &get_resource() const;
    void set_resource(const resource &text);

    const std::string &get_text() const;
    void set_text(const std::string &);

    const io::unix_file *get_file() const;
    void set_file(io::unix_file *file) noexcept;

    bool get_keep_alive() const noexcept;
    void set(const std::string &field, const std::string &value) noexcept;
    const std::vector<std::pair<std::string, std::string>> &get_fields() const noexcept;

    private:
    std::vector<std::pair<std::string, std::string>> fields_;
    version version_ = {1, 1};
    status_code code_;
    type type_;
    resource resource_;
    std::string text_;
    const io::unix_file *file_ = nullptr;
    void Init();
};
};

#endif // RESPONSE_H
