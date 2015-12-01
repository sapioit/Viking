#ifndef COMMON
#define COMMON
#include <experimental/optional>

#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#endif // COMMON
