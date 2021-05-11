#pragma once

#include <initializer_list>

#include <libmath/MinMax.h>
#include <libutils/Assert.h>
#include <libutils/Iteration.h>

template <typename T, size_t N>
class Array
{
    static_assert(N > 0, "Array must have size greater than 0");

private:
    T _storage[N];

public:
    constexpr size_t count() const { return N; }

    constexpr T *raw_storage() { return _storage; }
    constexpr const T *raw_storage() const { return _storage; }

    T &at(size_t index)
    {
        assert(index < N);
        return _storage[index];
    }

    const T &at(size_t index) const
    {
        assert(index < N);
        return _storage[index];
    }

    constexpr Array()
    {
    }

    constexpr Array(std::initializer_list<T> data)
    {
        assert(data.size() <= N);

        for (size_t i = 0; i < MIN(N, data.size()); i++)
        {
            _storage[i] = *(data.begin() + i);
        }
    }

    T &operator[](size_t index)
    {
        assert(index < N);

        return _storage[index];
    }

    const T &operator[](size_t index) const
    {
        assert(index < N);

        return _storage[index];
    }

    bool operator!=(const Array &other) const
    {
        return !(*this == other);
    }

    bool operator==(const Array<T, N> &other) const
    {
        if (this == &other)
        {
            return true;
        }

        if (count() != other.count())
        {
            return false;
        }

        for (size_t i = 0; i < count(); i++)
        {
            if (_storage[i] != other._storage[i])
            {
                return false;
            }
        }

        return true;
    }

    Utils::ContiguousIterator<T> begin() const { return {_storage}; }
    Utils::ContiguousIterator<T> end() const { return {_storage + N}; }
};