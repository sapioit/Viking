#ifndef CACHE_POLICY
#define CACHE_POLICY

#include <cstdint>

namespace Http {
struct CachePolicy {
    std::uint32_t max_age = 0;
};
}

#endif // CACHE_POLICY
