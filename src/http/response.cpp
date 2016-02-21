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

status_code response::get_code() const { return code_; }
void response::set_code(status_code code) { code_ = code; }

response::type response::get_type() const { return type_; }
void response::set_type(response::type type) noexcept { type_ = type; }

const resource &response::get_resource() const { return resource_; }

const std::string &response::get_text() const { return text_; }
void response::set_text(const std::string &text) {
    text_ = text;
    type_ = type::text;
}

const io::unix_file *response::get_file() const { return file_; }
void response::set_file(io::unix_file *file) noexcept {
    file_ = file;
    type_ = type::file;
}

version response::get_version() const { return version_; }
void response::set_version(version version) { version_ = version; }

void response::set(const std::string &field, const std::string &value) noexcept {
    auto it = std::find_if(fields_.begin(), fields_.end(), [field](auto pair) { return pair.first == field; });

    if (it == fields_.end())
        fields_.emplace_back(std::make_pair(field, value));
    else
        it->second = value;
}

const std::vector<std::pair<std::string, std::string>> &response::get_fields() const noexcept { return fields_; }

using f = http::header::fields;

std::size_t response::content_len() const {
    auto it = std::find_if(get_fields().begin(), get_fields().end(), [](auto pair) { return pair.first == f::Content_Length; });
    if (it != get_fields().end())
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
    auto it = std::find_if(fields_.begin(), fields_.end(), [](auto pair) { return pair.first == f::Connection; });
    if (it != fields_.end() && it->second == "Keep-Alive")
        return true;
    return false;
}

void response::init() {
    fields_.reserve(256);
    set(f::Date, Date::Now().ToString());
    set(f::Connection, "Keep-Alive");
    set(f::Access_Control_Allow_Origin, "*");
    set(f::Content_Type, "text/plain; charset=utf-8");
    set(f::Content_Length, std::to_string(content_len()));
    set(f::Transfer_Encoding, "binary");
    set(f::Cache_Control, "max-age=" + std::to_string(storage::config().default_max_age));
}

response::response() : code_(status_code::OK) {
    type_ = type::text;
    init();
}

response::response(status_code code) : code_(code) {
    type_ = type::text;
    init();
}

response::response(const std::string &text) : code_(status_code::OK), text_({text.begin(), text.end()}) {
    type_ = type::text;
    init();
}

response::response(const resource &resource) : code_(status_code::OK), resource_(resource) {
    type_ = type::resource;
    init();
    set(f::Content_Type,
        http::Util::get_mimetype(resource.path())); // mime_types[(filesystem::GetExtension(resource.Path()))]);
}

response::response(resource &&resource) : code_(status_code::OK), resource_(std::move(resource)) {
    type_ = type::resource;
    init();
    set(f::Content_Type,
        http::Util::get_mimetype(resource.path())); // mime_types[(filesystem::GetExtension(resource.Path()))]);
}
