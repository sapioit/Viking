#ifndef RESOURCECACHE_H
#define RESOURCECACHE_H

#include <misc/resource.h>

namespace Cache {
class ResourceCache {
    public:
    static Resource Aquire(fs::path p);
};
}

#endif // RESOURCECACHE_H
