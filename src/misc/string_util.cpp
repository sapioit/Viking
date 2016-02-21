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

std::vector<std::string> split(const std::string &source, char delimiter) noexcept {
    std::vector<std::string> result;
    std::istringstream ss(source);
    std::string tok;
    while (std::getline(ss, tok, delimiter)) {
        if (!tok.empty())
            result.push_back(tok);
    }
    return result;
}

std::string char_to_hex(unsigned char c) {
    short i = c;
    std::stringstream s;
    s << "%" << std::setw(2) << std::setfill('0') << std::hex << i;
    return s.str();
}

unsigned char hex_to_char(const std::string &str) {
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

std::string url_decode(const std::string &to_decode) {
    std::ostringstream out;
    for (std::string::size_type i = 0; i < to_decode.length(); ++i) {
        if (to_decode.at(i) == '%') {
            std::string str(to_decode.substr(i + 1, 2));
            out << hex_to_char(str);
            i += 2;
        } else {
            out << to_decode.at(i);
        }
    }
    return out.str();
}
