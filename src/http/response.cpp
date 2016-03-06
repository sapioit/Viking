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
#include <http/response.h>
#include <http/request.h>
#include <http/util.h>
#include <io/filesystem.h>
#include <inl/mime_types.h>
#include <misc/date.h>
#include <misc/storage.h>
#include <misc/common.h>

#include <sstream>
#include <utility>
#include <iomanip>
#include <iterator>
#include <algorithm>

using namespace http;

status_code response::get_code() const noexcept { return code_; }
void response::set_code(status_code code) noexcept { code_ = code; }

response::type response::get_type() const noexcept { return type_; }
void response::set_type(response::type type) noexcept { type_ = type; }

const resource &response::get_resource() const { return resource_; }
void response::set_resource(const resource &r) noexcept { resource_ = r; }

const std::string &response::get_text() const noexcept { return text_; }
void response::set_text(const std::string &text) noexcept {
    text_ = text;
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

std::size_t response::content_len() const noexcept {
    auto it = fields.find(f::Content_Length);
    if (it != fields.end())
        return std::stoi(it->second);
    if (get_type() == type::file)
        return file_->size;
    if (get_type() == type::resource)
        return resource_.content().size();
    if (get_type() == type::text)
        return text_.size();
    return 0;
}

bool response::get_keep_alive() const noexcept {
    auto it = fields.find(f::Connection);
    return it != fields.end() ? it->second == "Keep-Alive" : false;
}

void response::init() {
    fields.reserve(7);
    set(f::Date, date::now().to_string());
    set(f::Connection, "Keep-Alive");
    set(f::Access_Control_Allow_Origin, "*");
    set(f::Content_Type, "text/plain; charset=utf-8");
    set(f::Content_Length, std::to_string(content_len()));
    set(f::Transfer_Encoding, "binary");
    set(f::Cache_Control, "max-age=" + std::to_string(storage::config().default_max_age));
    version = {1, 1};
}

response::response(bool cc) : code_(status_code::OK) {
    type_ = type::text;
    init();
    if (!cc)
        set(http::header::fields::Cache_Control, "no-cache");
}

response::response(status_code code) : code_(code) {
    type_ = type::text;
    init();
    if (code == http::status_code::NotFound || code == http::status_code::InternalServerError ||
        code == http::status_code::BadRequest)
        set(http::header::fields::Cache_Control, "no-cache");
}

response::response(const std::string &text, bool cc) : code_(status_code::OK), text_({text.begin(), text.end()}) {
    type_ = type::text;
    init();
    if (!cc)
        set(http::header::fields::Cache_Control, "no-cache");
}

response::response(const resource &resource, bool cc) : code_(status_code::OK), resource_(resource) {
    type_ = type::resource;
    init();
    set(f::Content_Type, http::util::get_mimetype(resource.path()));
    if (!cc)
        set(http::header::fields::Cache_Control, "no-cache");
}

response::response(resource &&resource, bool cc) : code_(status_code::OK), resource_(std::move(resource)) {
    type_ = type::resource;
    init();
    set(f::Content_Type, http::util::get_mimetype(resource.path()));
    if (!cc)
        set(http::header::fields::Cache_Control, "no-cache");
}
