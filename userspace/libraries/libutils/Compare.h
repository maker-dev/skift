#pragma once

#include <libutils/Traits.h>

namespace Utils
{
template <class T, class Compare>
inline T min(T &a, T &b, Compare comp)
{
    return (comp(b, a)) ? b : a;
}

template <class T, class Compare>
inline T max(T &a, T &b, Compare comp)
{
    return (comp(a, b)) ? b : a;
}

template <class T, class Compare>
inline T median_of_three(const T &a, const T &b, const T &c, Compare comp)
{
    return max(min(a, b, comp), min(max(a, b, comp), c, comp), comp);
}

} // namespace Utils
