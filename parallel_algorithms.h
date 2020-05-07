#ifndef __PARALLEL_ALGORITHMS_H
#define __PARALLEL_ALGORITHMS_H

#include <omp.h>
#include <random>
#include <parallel/algorithm>

#include "parallel_array.h"
#include "prefix_sum.h"

// TODO write my own qsort implementation
template<typename It>
void parallel_sort(It begin, It end) {
    __gnu_parallel::sort(begin, end);
}

#endif
