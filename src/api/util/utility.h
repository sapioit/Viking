#include <utility>

namespace Utility {
template <typename T1, typename T2>
std::pair<T1 *, T2 *> Merge(const T1 &f, T2 &s) {
        return std::make_pair(std::addressof(f), std::addressof(s));
}
}
