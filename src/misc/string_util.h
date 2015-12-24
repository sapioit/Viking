#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <string>
#include <misc/date.h>

class StringUtil {
    public:
    static std::vector<std::string> Split(const std::string &, char) noexcept;
    static std::string DecodeURL(const std::string &sSrc);
};

#endif // UTIL_H
