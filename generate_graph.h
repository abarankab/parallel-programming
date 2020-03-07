#ifndef __GENERATE_GRAPH_H
#define __GENERATE_GRAPH_H

#include <iostream>
#include <omp.h>
#include <random>

#include "defs.h"

/**
* INTERFACE:
* RandomSequence(uint32_t size, uint32_t master_seed) - constructs a random sequence of given size
* uint32_t operator[i] const - ith number
*/
struct RandomSequence {
    const u32 NUM_THREADS;
    u32 size;
    u32* arr;

    RandomSequence(u32 size, u32 NUM_THREADS = omp_get_max_threads()) : NUM_THREADS(NUM_THREADS), size(size) {
        arr = static_cast<u32*>(operator new[] (size * sizeof(u32)));

        #pragma omp parallel shared(arr)
        {
            std::mt19937 gen(std::random_device{}());
            #pragma omp for
            for (u32 i = 0; i < size; ++i) {
                arr[i] = gen();
            }
        }
    }

    ~RandomSequence() {
        delete[] arr;
    }

    u32 operator[](u32 id) {
        return arr[id];
    }
};



#endif
