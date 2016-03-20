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
#include <cache/resource_cache.h>
#include <misc/common.h>

resource cache::resource_cache::aquire(fs::path p) {
    static std::unordered_map<fs::path, resource> storage;

    auto r = storage.find(p);
    if (!fs::exists(p) && r != storage.end()) {
        storage.erase(r);
        return {};
    }
    if (r != storage.end()) {
        if (unlikely(fs::last_write_time(p) > r->second.last_write()))
            return storage[p] = resource{p};
        else
            return r->second;
    } else {
        return storage[p] = resource{p};
    }
}
