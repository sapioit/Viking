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

using namespace Http;

StatusCode Response::GetCode() const { return code_; }
void Response::SetCode(StatusCode code) { code_ = code; }

Response::Type Response::GetType() const { return type_; }
void Response::SetType(Response::Type type) noexcept { type_ = type; }

const resource &Response::GetResource() const { return resource_; }

const std::string &Response::GetText() const { return text_; }
void Response::SetText(const std::string &text) {
    text_ = text;
    type_ = Type::Text;
}

const io::unix_file *Response::GetFile() const { return file_; }
void Response::SetFile(io::unix_file *file) noexcept {
    file_ = file;
    type_ = Type::File;
}

Version Response::GetVersion() const { return version_; }
void Response::SetVersion(Version version) { version_ = version; }

void Response::Set(const std::string &field, const std::string &value) noexcept {
    auto it = std::find_if(fields_.begin(), fields_.end(), [field](auto pair) { return pair.first == field; });

    if (it == fields_.end())
        fields_.emplace_back(std::make_pair(field, value));
    else
        it->second = value;
}

const std::vector<std::pair<std::string, std::string>> &Response::GetFields() const noexcept { return fields_; }

using f = Http::Header::Fields;

std::size_t Response::ContentLength() const {
    auto it = std::find_if(fields_.begin(), fields_.end(), [](auto pair) { return pair.first == f::Content_Length; });
    if (it != fields_.end())
        return std::stoi(it->second);
    if (GetType() == Type::File)
        return file_->size;
    if (GetType() == Type::Resource)
        return resource_.content().size();
    if (GetType() == Type::Text)
        return text_.size();
    return 0;
}

bool Response::GetKeepAlive() const noexcept {
    auto it = std::find_if(fields_.begin(), fields_.end(), [](auto pair) { return pair.first == f::Connection; });
    if (it != fields_.end() && it->second == "Keep-Alive")
        return true;
    return false;
}

void Response::Init() {
    fields_.reserve(256);
    Set(f::Date, Date::Now().ToString());
    Set(f::Connection, "Keep-Alive");
    Set(f::Access_Control_Allow_Origin, "*");
    Set(f::Content_Type, "text/plain; charset=utf-8");
    Set(f::Content_Length, std::to_string(ContentLength()));
    Set(f::Transfer_Encoding, "binary");
    Set(f::Cache_Control, "max-age=" + std::to_string(Storage::GetSettings().default_max_age));
}

Response::Response() : code_(StatusCode::OK) {
    type_ = Type::Text;
    Init();
}

Response::Response(StatusCode code) : code_(code) {
    type_ = Type::Text;
    Init();
}

Response::Response(const std::string &text) : code_(StatusCode::OK), text_({text.begin(), text.end()}) {
    type_ = Type::Text;
    Init();
}

Response::Response(const resource &resource) : code_(StatusCode::OK), resource_(resource) {
    type_ = Type::Resource;
    Init();
    Set(f::Content_Type,
        Http::Util::get_mimetype(resource.path())); // mime_types[(filesystem::GetExtension(resource.Path()))]);
}

Response::Response(resource &&resource) : code_(StatusCode::OK), resource_(std::move(resource)) {
    type_ = Type::Resource;
    Init();
    Set(f::Content_Type,
        Http::Util::get_mimetype(resource.path())); // mime_types[(filesystem::GetExtension(resource.Path()))]);
}
