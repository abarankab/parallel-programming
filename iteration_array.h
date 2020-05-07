#ifndef __ITERATION_ARRAY_H
#define __ITERATION_ARRAY_H

#include <bitset>
#include <list>

#include "defs.h"
#include "parallel_array.h"

template<typename T>
struct IterationArray {
    ParallelArray<T> arr;
    ParallelArray<bool> is_empty;
    std::list<int> non_empty;

    IterationArray(u32 size) : arr(size), is_empty(size) {}

    const T& operator[](u32 id) {
        return arr[id];
    }

    T& operator[](u32 id) {

    }
};

#endif
