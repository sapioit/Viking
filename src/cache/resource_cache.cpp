#include <cache/resource_cache.h>
#include <misc/common.h>
#include <unordered_map>

namespace std {
template <> struct hash<fs::path> {
    typedef fs::path argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const &s) const { return std::hash<std::string>{}(s); }
};
}

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
