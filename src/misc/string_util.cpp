#include <misc/string_util.h>
#include <sstream>

std::vector<std::string> StringUtil::Split(const std::string &source, char delimiter) noexcept {
    std::vector<std::string> result;
    std::istringstream ss(source);
    std::string tok;
    while (std::getline(ss, tok, delimiter)) {
        if (!tok.empty())
            result.push_back(tok);
    }
    return result;
}
