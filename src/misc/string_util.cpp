/*
Copyright (C) 2015 Voinea Constantin Vladimir

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/
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
