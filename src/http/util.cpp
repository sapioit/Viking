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
#include <http/util.h>
#include <io/filesystem.h>
#include <inl/mime_types.h>
#include <misc/storage.h>
#include <misc/common.h>
using namespace http;

static std::string exec(const std::string &cmd) {
    std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
        return "";
    char buffer[128];
    std::string result = "";
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    return result;
}

bool util::is_passable(const http::request &request) noexcept {
    switch (request.method) {
    case http::method::Get:
        return true;
    case http::method::Post:
        return true;
    case http::method::Put:
        return true;
    case http::method::Delete:
        return true;
    case http::method::Head:
        return true;
    default:
        return false;
    }
    return true;
}

bool util::is_disk_resource(const request &request) noexcept {
    fs::path full_path = storage::config().root_path + request.url;
    return fs::exists(full_path);
}

bool util::is_complete(const request &request) noexcept {
    if (can_have_body(request.method)) {
        auto cl_it = request.m_header.get_fields_c().find(http::header::fields::Content_Length);
        if (cl_it != request.m_header.get_fields_c().end()) {
            auto content_length = static_cast<std::size_t>(std::atoi(cl_it->second.c_str()));
            if (request.body.size() < content_length)
                return false;
        }
        return true;
    }
    return true;
}

bool util::can_have_body(method method) noexcept {
    switch (method) {
    case http::method::Put:
        return true;
    case http::method::Post:
        return true;
    case http::method::Options:
        return true;
    default:
        return false;
    }
}

static std::string shell_get_mimetype(fs::path p) noexcept {
    std::string cmd = "file --mime-type ";
    cmd.append(p);
    auto output = exec(cmd);
    while (output.size() && std::isspace(output.back()))
        output = output.substr(0, output.size() - 1);
    auto c = output.find_first_of(':');
    if (c != std::string::npos && output.size() > ++c)
        return output.substr(c + 1);
    return "";
}

std::string util::get_mimetype(fs::path p) noexcept {
    auto ext = io::get_extension(p);
    if (ext.length())
        return mime_types[ext];
    else
        return shell_get_mimetype(p);
}
