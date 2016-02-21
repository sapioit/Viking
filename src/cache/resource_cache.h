#ifndef RESOURCECACHE_H
#define RESOURCECACHE_H

#include <misc/resource.h>

namespace cache {
class resource_cache {
    public:
    static resource aquire(fs::path p);
};
}

#endif // RESOURCECACHE_H
