#pragma once

#include <libutils/Traits.h>

namespace Utils
{
template <class Iterator>
inline void advance(Iterator &it, int64_t n = 1)
{
    while (n > 0)
    {
        --n;
        ++it;
    }
    while (n < 0)
    {
        ++n;
        --it;
    }
}

template <class Iterator>
inline Iterator prev(Iterator it, size_t n = 1)
{
    advance(it, -n);
    return it;
}

template <class Iterator>
inline Iterator next(Iterator it, size_t n = 1)
{
    advance(it, n);
    return it;
}

template <class Iterator>
inline size_t distance(Iterator first, Iterator last)
{
    size_t result = 0;
    while (first != last)
    {
        ++first;
        ++result;
    }
    return result;
}

template <class T>
class ContiguousIterator
{
public:
    ContiguousIterator(T *ptr) : _ptr(ptr) {}
    ContiguousIterator operator++()
    {
        ++_ptr;
        return *this;
    }
    ContiguousIterator operator--()
    {
        --_ptr;
        return *this;
    }

    T &operator->()
    {
        return *_ptr;
    }

    bool operator!=(const ContiguousIterator<T> &other) const { return _ptr != other._ptr; }
    const T &operator*() const { return *_ptr; }

private:
    T *_ptr;
};

} // namespace Utils
