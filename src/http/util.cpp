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
#include <misc/storage.h>
using namespace Http;

bool Util::IsPassable(const Http::Request &request) noexcept {
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

bool Util::IsResource(const Request &request) noexcept {
    fs::path full_path = Storage::GetSettings().root_path + request.url;
    if (fs::exists(full_path) && fs::is_regular_file(full_path)) {
        if (filesystem::GetExtension(full_path) != "")
            return true;
    }
    return false;
}

bool Util::IsComplete(const Request &request) noexcept {
    if (CanHaveBody(request.method)) {
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

bool Util::CanHaveBody(Method method) noexcept {
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
