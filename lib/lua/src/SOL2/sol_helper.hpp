#ifndef SOL_HELPER_HPP
#define SOL_HELPER_HPP

#include <vector>
#include "sol.hpp"

// customized from https://stackoverflow.com/a/40777268/16456439
/**
 * Convert a Lua sequence into a C++ vector
 * Throw exception on errors or wrong types
 */
template <typename elementType>
std::vector<elementType> convert_sequence(sol::table t)
{
    std::size_t sz = t.size();
    std::vector<elementType> res(sz);
    for (int i = 1; i <= sz; i++) {
        res[i - 1] = t[i].get<elementType>();
    }
    return res;
}

#endif // SOL_HELPER_HPP