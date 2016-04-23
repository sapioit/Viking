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
#include <http/header.h>
#include <http/request.h>
#include <http/response.h>
#include <http/util.h>
#include <inl/mime_types.h>
#include <io/filesystem.h>
#include <misc/common.h>
#include <misc/compression.h>
#include <misc/date.h>
#include <misc/debug.h>
#include <misc/storage.h>
#include <misc/string_util.h>

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <utility>

using namespace http;

status_code response::get_code() const noexcept { return code_; }
void response::set_code(status_code code) noexcept { code_ = code; }

response::type response::get_type() const noexcept { return type_; }
void response::set_type(response::type type) noexcept { type_ = type; }

const resource &response::get_resource() const { return res; }
void response::set_resource(const resource &r) noexcept { res = r; }

const std::vector<char> &response::get_text() const noexcept { return text_; }
void response::set_text(const std::string &text) noexcept {
    text_ = {text.cbegin(), text.cend()};
    type_ = type::text;
}

const io::unix_file *response::get_file() const noexcept { return file_; }
void response::set_file(io::unix_file *file) noexcept {
    file_ = file;
    type_ = type::file;
}

response &response::set(const std::string &field, const std::string &value) noexcept {
    fields[field] = value;
    return *this;
}

using f = http::header::fields;

bool response::get(const std::string &str, std::string &target, bool from_req) const noexcept {
    if (from_req) {
        auto it = fields.find(str);
        if (it != fields.end()) {
            target = it->second;
            return true;
        }
        return false;
    } else {
        auto it = req.m_header.get_fields_c().find(str);
        if (it != req.m_header.get_fields_c().end()) {
            target = it->second;
            return true;
        }
        return false;
    }
}

request response::get_request() const { return req; }

void response::set_request(const request &value) { req = value; }

bool response::body_available() const noexcept { return get_type() == type::resource || get_type() == type::text; }

const std::vector<char> &response::body() const {
    switch (get_type()) {
    case type::resource:
        if (compressed == compression_type::deflate)
            return res.deflated;
        if (compressed == compression_type::gzip)
            return res.gzipped;
        return res.raw;
        break;
    case type::text:
        return text_;
        break;
    case type::file:
        throw body_unavailable{};
    }
    throw body_unavailable{};
}

std::size_t response::content_len() const noexcept {
    if (get_type() == type::file)
        return file_->size;
    return body().size();
}

bool response::get_keep_alive() const noexcept {
    auto it = fields.find(f::Connection);
    return it != fields.end() ? uppercase(it->second) == "KEEP-ALIVE" : true;
}

void response::try_to_compress() noexcept {
    if (body_available() && compressed == compression_type::none) {
        switch (get_type()) {
        case type::resource:
            if (util::can_compress(req, "deflate")) {
                if (!res.deflated.size())
                    res.deflated = compression::deflate(res.raw);
                set(f::Content_Encoding, "deflate");
                compressed = compression_type::deflate;
            } else if (util::can_compress(req, "gzip")) {
                if (!res.gzipped.size())
                    res.gzipped = compression::gzip(res.raw);
                set(f::Content_Encoding, "gzip");
                compressed = compression_type::gzip;
            }
            break;
        case type::text:
            if (util::can_compress(req, "deflate")) {
                text_ = compression::deflate(text_);
                set(f::Content_Encoding, "deflate");
                compressed = compression_type::deflate;
            } else if (util::can_compress(req, "gzip")) {
                text_ = compression::gzip(text_);
                set(f::Content_Encoding, "gzip");
                compressed = compression_type::gzip;
            }
            break;
        default:
            break;
        }
    }
}

void response::init() {
    if (storage::config().enable_compression)
        try_to_compress();
    version = {1, 1};
    fields.reserve(7);
    set(f::Date, date::now().to_string());
    set(f::Access_Control_Allow_Origin, "*");
    set(f::Content_Type, "text/plain; charset=utf-8");
    set(f::Transfer_Encoding, "binary");

    std::string req_conn_status;
    if (get(f::Connection, req_conn_status, true))
        set(f::Connection, req_conn_status);
    else
        set(f::Connection, "Close");

    std::string req_cache_control;
    if (get(f::Cache_Control, req_cache_control, true) && req_cache_control.find("no-cache") == std::string::npos)
        set(f::Cache_Control, "max-age=" + std::to_string(storage::config().default_max_age));

    if (body_available())
        set(f::Content_Length, std::to_string(content_len()));
}

response::response(request r) : req(r), code_(status_code::OK), compressed(compression_type::none) {
    type_ = type::text;
    init();
}

response::response(request r, io::unix_file *file)
    : req(r), code_(status_code::OK), compressed(compression_type::none), file_(file) {
    type_ = type::file;
    init();
    if (file) {
        set(f::Content_Type, http::util::get_mimetype(file->path));
        set(f::Content_Length, std::to_string(file->size));
    }
}

response::response(request r, status_code code) : req(r), code_(code), compressed(compression_type::none) {
    type_ = type::text;
    init();
}

response::response(request r, const std::string &text)
    : req(r), code_(status_code::OK), text_({text.begin(), text.end()}), compressed(compression_type::none) {
    type_ = type::text;
    init();
}

response::response(request r, const resource &resource)
    : req(r), code_(status_code::OK), res(resource), compressed(compression_type::none) {
    type_ = type::resource;
    init();
    set(f::Content_Type, http::util::get_mimetype(resource.path()));
}

response &response::operator=(const std::string &str) {
    type_ = type::text;
    text_ = {str.cbegin(), str.cend()};
    init();
    return *this;
}

response &response::operator=(const resource &r) {
    type_ = type::resource;
    res = r;
    init();
    return *this;
}

response &response::operator=(status_code s) {
    type_ = type::text;
    code_ = s;
    init();
    return *this;
}
