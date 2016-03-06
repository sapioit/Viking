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
#include <unordered_map>
#include <future>

namespace http {
class response {
    public:
    enum class type { resource, file, text };
    response(bool = true);
    response(status_code);
    response(const std::string &, bool cc = true);
    response(http::status_code, const std::string &);
    response(const resource &, bool cc = true);
    response(resource &&, bool cc = true);
    virtual ~response() = default;

    status_code get_code() const noexcept;
    void set_code(status_code get_code) noexcept;

    std::size_t content_len() const noexcept;

    type get_type() const noexcept;
    void set_type(type type) noexcept;

    const resource &get_resource() const;
    void set_resource(const resource &text) noexcept;

    const std::string &get_text() const noexcept;
    void set_text(const std::string &) noexcept;

    const io::unix_file *get_file() const noexcept;
    void set_file(io::unix_file *file) noexcept;

    bool get_keep_alive() const noexcept;
    response &set(const std::string &field, const std::string &value) noexcept;

    std::unordered_map<std::string, std::string> fields;
    http_version version;

    private:
    status_code code_;
    type type_;
    resource resource_;
    std::string text_;
    const io::unix_file *file_ = nullptr;
    void init();
};
};

#endif // RESPONSE_H
