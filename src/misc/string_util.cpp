#include <misc/string_util.h>
#include <sstream>
#include <iomanip>

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

std::string charToHex(unsigned char c) {
    short i = c;

    std::stringstream s;

    s << "%" << std::setw(2) << std::setfill('0') << std::hex << i;

    return s.str();
}

unsigned char hexToChar(const std::string &str) {
    short c = 0;

    if (!str.empty()) {
        std::istringstream in(str);

        in >> std::hex >> c;

        if (in.fail()) {
            throw std::runtime_error("stream decode failure");
        }
    }

    return static_cast<unsigned char>(c);
}

std::string StringUtil::DecodeURL(const std::string &toDecode) {
    std::ostringstream out;

    for (std::string::size_type i = 0; i < toDecode.length(); ++i) {
        if (toDecode.at(i) == '%') {
            std::string str(toDecode.substr(i + 1, 2));
            out << hexToChar(str);
            i += 2;
        } else {
            out << toDecode.at(i);
        }
    }

    return out.str();
}
