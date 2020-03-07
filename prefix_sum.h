#include <omp.h>
#include <vector>

#include "defs.h"

/**
 * INTERFACE:
 * 
 * PrefixSum(uint32_t size, T* begin, uint32_t NUM_THREADS) - constructs an array of prefix sums using NUM_THREADS
 * T operator[i] - returns ith prefix sum = a[0] + ... + a[i]
 */
template<typename T>
struct PrefixSum {
    const u32 NUM_THREADS;
    u32 size;
    T* prefix_sum;

    template<typename U>
    PrefixSum(u32 size, U* arr, u32 NUM_THREADS = omp_get_max_threads()) : NUM_THREADS(NUM_THREADS), size(size) {
        prefix_sum = static_cast<T*>(operator new[] (size * sizeof(T)));
        std::vector<T> thread_sum(NUM_THREADS);

        #pragma omp parallel shared(thread_sum) num_threads(NUM_THREADS)
        {
            u32 thread_num = omp_get_thread_num();

            T current_sum = 0;

            #pragma omp for schedule(static) nowait
            for (u32 i = 0; i < size; ++i) {
                current_sum += arr[i];
                prefix_sum[i] = current_sum;
            }
            thread_sum[thread_num] = current_sum;
            #pragma omp barrier

            T offset = 0;
            for (u32 i = 0; i < thread_num; ++i) {
                offset += thread_sum[i];
            }

            #pragma omp for schedule(static)
            for (u32 i = 0; i < size; ++i) {
                prefix_sum[i] += offset;
            }
        }
    }

    ~PrefixSum() {
        delete[] prefix_sum;
    }

    T operator[](u32 id) {
        return prefix_sum[id];
    }
};