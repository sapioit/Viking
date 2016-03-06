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
