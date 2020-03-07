#include <immintrin.h>
#include <smmintrin.h>
#include <malloc.h>
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
    const u32 NUM_THREADS;
    u32 size;
    u32 initial_size;
    u32* prefix_sum;

    /* http://ppc.cs.aalto.fi/ch2/v3/ */
    u32* u32_alloc(u32 size) {
        void* ptr = 0;
        if (posix_memalign(&ptr, 8, size * sizeof(u32))) {
            throw std::bad_alloc();
        }
        return static_cast<u32*>(ptr);
    }

    PrefixSum(u32 size, u32* arr, u32 NUM_THREADS = omp_get_max_threads()) : NUM_THREADS(NUM_THREADS),
                                                                             size(size),
                                                                             initial_size(size) {
        size += (size % 8 == 0) ? (0) : (8 - (size % 8));
        prefix_sum = u32_alloc(size);
        for (u32 i = initial_size; i < size; ++i) prefix_sum[i] = 0;

        std::vector<u32> thread_sum(NUM_THREADS);

        #pragma omp parallel num_threads(1)
        {
            u32 thread_num = omp_get_thread_num();
            u32 current_sum = 0;

            #pragma omp for schedule(static) nowait
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

            __m256i_u vec_offset = _mm256_set1_epi32(offset);
            std::cout << "set1\n";
            #pragma omp for schedule(static)
            for (u32 i = 0; i < size / 8; ++i) {
                __m256i_u chunk = _mm256_load_si256((__m256i_u*)(&prefix_sum[8 * i]));
                std::cout << "load\n";
                chunk = _mm256_add_epi32(chunk, vec_offset);
                std::cout << "add\n";
                _mm256_store_si256((__m256i*)&prefix_sum[8 * i], chunk);
                std::cout << "store\n";
            }
        }
    }

    ~PrefixSum() {
        free(prefix_sum);
    }

    u32 operator[](u32 id) {
        if (id >= initial_size) throw std::out_of_range("Prefix sum index out of range");
        return prefix_sum[id];
    }
};