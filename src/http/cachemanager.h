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
#ifndef CACHEMANAGER_H
#define CACHEMANAGER_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <misc/resource.h>

class CacheManager {
    static std::unordered_map<std::string, Resource> _resources;
    static std::mutex _cacheLock;

    public:
    struct FileTooBig {};
    static Resource GetItem(const std::string &);
    static void PutItem(const std::pair<std::string, Resource> &&);
    static void ReplaceItem(const std::string &, const Resource &);
    static Resource GetResource(const std::string &);
};

#endif // CACHEMANAGER_H
