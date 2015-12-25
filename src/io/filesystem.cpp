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
#include <io/filesystem.h>
#include <misc/debug.h>
#include <misc/common.h>
#include <sys/stat.h>
#include <fstream>
#include <unistd.h>
using namespace IO;

std::vector<char> FileSystem::ReadFile(const std::string &path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream.is_open())
        throw fs_error("File could not be opened");
    std::vector<char> fileContents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    return fileContents;
}

std::string FileSystem::GetCurrentDirectory() {
    char *cwd = NULL;
    cwd = getcwd(0, 0);
    std::string cur_dir(cwd);
    free(cwd);

    return cur_dir;
}

std::size_t FileSystem::GetFileSize(const std::string &file_path) {
    struct stat64 st;
    if (-1 == ::stat64(file_path.c_str(), &st)) {
        throw fs_error{"not found"};
    } else
        return st.st_size;
}

bool FileSystem::FileExists(const std::string &path) noexcept {
    struct stat64 st;
    return (-1 != ::stat64(path.c_str(), &st));
}

bool FileSystem::IsRegularFile(const std::string &path) noexcept {
    struct stat64 st;
    if (-1 != ::stat64(path.c_str(), &st))
        return S_ISREG(st.st_mode);
    return false;
}

std::string FileSystem::GetExtension(const std::string &path) noexcept {
    auto dot = path.find_last_of('.');
    auto slash = path.find_last_of('/');
    if (dot != std::string::npos) {
        if (unlikely(dot == 0))
            return "";
        std::string ext(path.begin() + dot + 1, path.end());
        if (slash != std::string::npos)
            return slash < dot ? ext : "";
        else
            return ext;
    }
    return "";
}
