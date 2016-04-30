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
#ifndef COMMON
#define COMMON
#include <type_traits>

#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#include <experimental/filesystem>
#include <unordered_map>
namespace fs = std::experimental::filesystem;
namespace std {
template <> struct hash<fs::path> {
    typedef fs::path argument_type;
    typedef std::size_t result_type;
    result_type operator()(const argument_type &p) const { return std::hash<std::string>()(p.string()); }
};
}
// namespace std {
// template <class E> class hash {
//    using sfinae = typename std::enable_if<std::is_enum<E>::value, E>::type;

//    public:
//    size_t operator()(const E &e) const { return std::hash<typename std::underlying_type<E>::type>()(e); }
//};
//};

#endif // COMMON
