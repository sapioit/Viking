#ifndef UTIL_H
#define UTIL_H
#include <http/request.h>

namespace Http {
class Util {
    public:
    static bool IsPassable(const Request &) noexcept;
    static bool ExtensionAllowed(const std::string &) noexcept;
};
}

#endif // UTIL_H
