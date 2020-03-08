#include <iostream>
#include <omp.h>
#include <random>

#include "../benchmark.h"
#include "../defs.h"
#include "../prefix_sum.h"
#include "../timer.h"

struct SequentialPrefixSum {
    u32 size;
    u32* prefix_sum;

    template<typename U>
    SequentialPrefixSum(u32 size, U* arr) : size(size) {
        prefix_sum = static_cast<u32*>(operator new[] (size * sizeof(u32)));
        prefix_sum[0] = arr[0];

        for (u32 i = 1; i < size; ++i) {
            prefix_sum[i] = prefix_sum[i - 1] + arr[i];
        }
    }

    ~SequentialPrefixSum() {
        delete[] prefix_sum;
    }

    u32 operator[](u32 id) {
        return prefix_sum[id];
    }
};

std::mt19937 gen(std::random_device{}());

u32 randint(u32 l, u32 r) {
    return std::uniform_int_distribution<u32>(l, r)(gen);
}

const u32 SMALL_NUM_STEPS = 10'000;
const u32 SMALL_SIZE = 100;

const u32 NUM_STEPS = 1'000;
const u32 MAX_SIZE = 100'000;

const u32 PERF_NUM_STEPS = 50;
const u32 PERF_SIZE = 10'000'000;

void check_correctness() {
    std::cout << std::fixed << "Checking correctness:\n";
    std::cout << "Checking small sizes:\n";
    for (u32 size = 1; size <= SMALL_SIZE; ++size) {
        std::cout << "Size " << size << "\n";
        for (u32 step = 1; step <= SMALL_NUM_STEPS; ++step) {
            u32* arr = new u32[size];
            for (u32 i = 0; i < size; ++i) {
                arr[i] = randint(1, 10);
            }

            SequentialPrefixSum correct(size, arr);
            PrefixSum to_check(size, arr);

            for (u32 i = 0; i < size; ++i) {
                if (correct[i] != to_check[i]) {
                    std::cerr << "Prefix sum mismatch at position " << i << ":\n";
                    std::cerr << "Expected " << correct[i] << " got " << to_check[i] << "\n";
                    std::cerr << "Array:\n";
                    for (u32 i = 0; i < size; ++i) {
                        std::cerr << arr[i] << " ";
                    }
                    std::cerr << "\nCorrect:\n";
                    for (u32 i = 0; i < size; ++i) {
                        std::cerr << correct[i] << " ";
                    }
                    std::cerr << "\nIncorrect:\n";
                    for (u32 i = 0; i < size; ++i) {
                        std::cerr << to_check[i] << " ";
                    }
                    std::cerr << "\n";
                    exit(-1);
                }
            }

            delete[] arr;
        }
    }
    std::cout << "OK\n";

    std::cout << "Checking on random sizes:\n";
    for (u32 step = 1; step <= NUM_STEPS; ++step) {
        if (step % (NUM_STEPS / 10) == 1) {
            std::cout << "Step " << step / (NUM_STEPS / 10) + 1 << " of 10\n";
        }

        u32 size = randint(1, MAX_SIZE);
        u32* arr = new u32[size];
        for (u32 i = 0; i < size; ++i) {
            arr[i] = randint(1, 10);
        }

        SequentialPrefixSum correct(size, arr);
        PrefixSum to_check(size, arr);

        for (u32 i = 0; i < size; ++i) {
            if (correct[i] != to_check[i]) {
                std::cerr << "Prefix sum mismatch at position " << i << ":\n";
                std::cerr << "Expected " << correct[i] << " got " << to_check[i] << "\n";
                std::cerr << "Array:\n";
                for (u32 i = 0; i < size; ++i) {
                    std::cerr << arr[i] << " ";
                }
                std::cerr << "\nCorrect:\n";
                for (u32 i = 0; i < size; ++i) {
                    std::cerr << correct[i] << " ";
                }
                std::cerr << "\nIncorrect:\n";
                for (u32 i = 0; i < size; ++i) {
                    std::cerr << to_check[i] << " ";
                }
                std::cerr << "\n";
                exit(-1);
            }
        }

        delete[] arr;
    }
    std::cout << "OK\n";
}

void check_performance() {
    u64 sequential_time = 0;
    u64 parallel_time = 0;

    for (u32 step = 1; step <= PERF_NUM_STEPS; ++step) {
        if (step % (PERF_NUM_STEPS / 10) == 1) {
            std::cout << "Step " << step / (PERF_NUM_STEPS / 10) + 1 << " of 10\n";
        }

        u32* arr = new u32[PERF_SIZE];
        for (u32 i = 0; i < PERF_SIZE; ++i) {
            arr[i] = gen();
        }

        {
            escape(&arr);
            u64 start = currentSeconds();
            SequentialPrefixSum sequental(PERF_SIZE, arr);
            u64 finish = currentSeconds();
            escape(&sequental);
            sequential_time += finish - start;
        }
        
        {
            escape(&arr);
            u64 start = currentSeconds();
            PrefixSum parallel(PERF_SIZE, arr);
            u64 finish = currentSeconds();
            escape(&parallel);
            parallel_time += finish - start;
        }

        delete[] arr;
    }

    std::cout << std::fixed;
    std::cout << "Sequential average time: "
              << static_cast<double>(sequential_time) / PERF_NUM_STEPS
              << "\nPer element: "
              << static_cast<double>(sequential_time) / PERF_NUM_STEPS / PERF_SIZE << "\n";
    std::cout << "Parallel average time: "
              << static_cast<double>(parallel_time) / PERF_NUM_STEPS
              << "\nPer element: "
              << static_cast<double>(parallel_time) / PERF_NUM_STEPS / PERF_SIZE << "\n";
}

int main() {
    check_correctness();
    check_performance();
}