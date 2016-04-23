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
#include <http/request.h>
#include <http/version.h>
#include <inl/status_codes.h>
#include <io/buffers/unix_file.h>
#include <misc/resource.h>

#include <future>
#include <string>
#include <unordered_map>

namespace http {
class response {
    public:
    struct body_unavailable {};
    enum class type { resource, file, text };
    enum class compression_type { deflate, gzip, none };
    response() = delete;
    response(request);
    response(request, io::unix_file *);
    response(request, status_code);
    response(request, const std::string &);
    response(request, http::status_code, const std::string &);
    response(request, const resource &);
    response &operator=(const std::string &);
    response &operator=(const resource &);
    response &operator=(status_code);
    virtual ~response() = default;

    status_code get_code() const noexcept;
    void set_code(status_code get_code) noexcept;

    std::size_t content_len() const noexcept;

    type get_type() const noexcept;
    void set_type(type type) noexcept;

    const resource &get_resource() const;
    void set_resource(const resource &text) noexcept;

    const std::vector<char> &get_text() const noexcept;
    void set_text(const std::string &) noexcept;

    const io::unix_file *get_file() const noexcept;
    void set_file(io::unix_file *file) noexcept;

    bool get_keep_alive() const noexcept;
    response &set(const std::string &field, const std::string &value) noexcept;
    std::string get(const std::string &, bool = false) const noexcept;

    std::unordered_map<std::string, std::string> fields;
    http_version version;

    request get_request() const;
    void set_request(const request &value);

    bool body_available() const noexcept;
    const std::vector<char> &body() const;

    private:
    request req;
    status_code code_;
    type type_;
    resource res;
    std::vector<char> text_;
    compression_type compressed;
    const io::unix_file *file_ = nullptr;
    void init();
    void try_to_compress() noexcept;
};
};

#endif // RESPONSE_H
