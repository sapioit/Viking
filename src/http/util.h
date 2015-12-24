#ifndef UTIL_H
#define UTIL_H
#include <http/request.h>

namespace Http {
class Util {
    public:
    static bool IsPassable(const Request &) noexcept;
    static bool ExtensionAllowed(const std::string &) noexcept;
    static bool IsComplete(const Http::Request &) noexcept;
    static bool CanHaveBody(Http::Method) noexcept;
};
}

#endif // UTIL_H
