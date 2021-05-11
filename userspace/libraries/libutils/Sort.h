#pragma once

#include <libutils/Compare.h>
#include <libutils/Iterator.h>
#include <libutils/Move.h>

namespace Utils
{
template <typename Iterator, typename Compare>
Iterator partition(Iterator first, Iterator pivot, Compare comp)
{
    auto pivPos = first;
    while (first != pivot)
    {
        if (comp(*first, *pivot))
        {
            swap(*first, *pivPos);
            ++pivPos;
        }
        ++first;
    }
    //last position of element which is not < as the pivot
    return pivPos;
}

template <class Iterator, class Compare>
inline void quicksort(Iterator first, Iterator last, Compare comp)
{
    auto dist = distance(first, last);

    // No need to sort
    if (dist < 2)
    {
        return;
    }

    auto pivot = prev(last);

    swap(*median_of_three(first, next(first, dist / 2), prev(last), comp), *pivot);

    auto pivPos = partition(first, pivot, comp);
    // Move pivot to correct position
    swap(*pivPos, *pivot);

    // Divide and conquer
    quicksort(first, pivPos, comp);
    quicksort(next(pivPos), last, comp);
}

} // namespace Utils
