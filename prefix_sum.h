#include <immintrin.h>
#include <malloc.h>
#include <memory>
#include <new>
#include <omp.h>
#include <stdexcept>
#include <stdlib.h>
#include <vector>

#include "defs.h"

/**
 * INTERFACE:
 * 
 * PrefixSum(uint32_t size, uint32_t* begin, uint32_t NUM_THREADS) - constructs an array of prefix sums using NUM_THREADS
 * uint32_t operator[i] - returns ith prefix sum = a[0] + ... + a[i]
 */
struct PrefixSum {
    const u32 VEC_CAP = 8;
    const u32 NUM_THREADS;

    u32 size;
    u32 initial_size;
    u32* prefix_sum;

    PrefixSum(u32 size, u32* arr, u32 NUM_THREADS = omp_get_max_threads()) : NUM_THREADS(NUM_THREADS),
                                                                             size(size),
                                                                             initial_size(size) {
        u32 lcm = VEC_CAP * NUM_THREADS;
        size = (size + lcm - 1) / lcm * lcm;

        prefix_sum = static_cast<u32*>(operator new[] (size * sizeof(u32),                   // array size
                                                       static_cast<std::align_val_t>(256)));  // alignment
        for (u32 i = initial_size; i < size; ++i) prefix_sum[i] = 0;

        std::vector<u32> thread_sum(NUM_THREADS);

        #pragma omp parallel num_threads(NUM_THREADS)
        {
            u32 thread_num = omp_get_thread_num();
            u32 current_sum = 0;

            #pragma omp for schedule(static, size / NUM_THREADS) nowait
            for (u32 i = 0; i < initial_size; ++i) {
                current_sum += arr[i];
                prefix_sum[i] = current_sum;
            }
            thread_sum[thread_num] = current_sum;
            #pragma omp barrier

            u32 offset = 0;
            for (u32 i = 0; i < thread_num; ++i) {
                offset += thread_sum[i];
            }

            __builtin_assume_aligned(prefix_sum, 256);
            #pragma omp for simd schedule(static, size / NUM_THREADS)
            for (u32 i = 0; i < initial_size; ++i) {
                prefix_sum[i] += offset;
            }
        }
    }

    ~PrefixSum() {
        delete[] prefix_sum;
    }

    u32 operator[](u32 id) {
        if (id >= initial_size) throw std::out_of_range("Prefix sum index out of range");
        return prefix_sum[id];
    }
};