#include <cache/resource_cache.h>
#include <misc/common.h>
#include <map>

Resource Cache::ResourceCache::Aquire(fs::path p) {
    static std::map<fs::path, Resource> map;

    auto r = map.find(p);
    if (!fs::exists(p) && r != map.end()) {
        map.erase(r);
        return {};
    }
    if (r != map.end()) {
        if (unlikely(fs::last_write_time(p) > r->second.LastWrite()))
            return map[p] = Resource{p};
        else
            return r->second;
    } else {
        return map[p] = Resource{p};
    }
}
