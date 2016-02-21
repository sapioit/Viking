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
using namespace Http;

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

bool Util::is_passable(const Http::Request &request) noexcept {
    switch (request.method) {
    case Http::Method::Get:
        return true;
    case Http::Method::Post:
        return true;
    case Http::Method::Put:
        return true;
    case Http::Method::Delete:
        return true;
    case Http::Method::Head:
        return true;
    default:
        return false;
    }
    return true;
}

bool Util::is_disk_resource(const Request &request) noexcept {
    fs::path full_path = Storage::GetSettings().root_path + request.url;
    if (fs::exists(full_path))
        return true;
    return false;
}

bool Util::is_complete(const Request &request) noexcept {
    if (can_have_body(request.method)) {
        auto cl_it = request.header.fields.find(Http::Header::Fields::Content_Length);
        if (cl_it != request.header.fields.end()) {
            auto content_length = static_cast<std::size_t>(std::atoi(cl_it->second.c_str()));
            if (request.body.size() < content_length)
                return false;
        }
        return true;
    }
    return true;
}

bool Util::can_have_body(Method method) noexcept {
    switch (method) {
    case Http::Method::Put:
        return true;
    case Http::Method::Post:
        return true;
    case Http::Method::Options:
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

std::string Util::get_mimetype(fs::path p) noexcept {
    auto ext = filesystem::get_extension(p);
    if (ext.length())
        return mime_types[ext];
    else
        return shell_get_mimetype(p);
}
