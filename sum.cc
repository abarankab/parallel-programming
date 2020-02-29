#include <iostream>
#include <stdlib.h>
#include <vector>
#include <omp.h>
#include <random>
#include <assert.h>

#include "benchmark.h"
#include "defs.h"
#include "timer.h"

std::vector<ull> init_data() {
    const ull MAX_SIZE = 100'000'000;
    const ull MAX_ELEMENT = 1'000'000;

    std::vector<ull> data(rand() % MAX_SIZE);
    for (ull& x : data) {
        x = rand() % MAX_ELEMENT;
    }
    return data;
}

ull par_sum(std::vector<ull>& v) {
    omp_set_num_threads(omp_get_max_threads());

    ull res = 0;
    size_t i = 0;

    #pragma omp parallel for reduction(+:res)
    for (i = 0; i < v.size(); ++i) {
        res += v[i];
    }
    return res;
}

ull seq_sum(std::vector<ull>& v) {
    ull res = 0;
    for (auto x : v) res += x;
    return res;
}

int main() {
    const size_t NUM_CYCLES = 1'00;
    const size_t CYCLE_STOP = NUM_CYCLES / 10;

    ull par_time = 0;
    ull seq_time = 0;

    std::cout << "Start\n" << "Threads: " << omp_get_max_threads() << "\n\n";

    for (size_t cycle = 1; cycle <= NUM_CYCLES; ++cycle) {
        std::vector<ull> data = init_data();
        escape(&data);

        ull par_result, seq_result;

        {
            ull start_time = currentSeconds();
            par_result = par_sum(data);
            ull end_time = currentSeconds();
            par_time += (end_time - start_time);
        }

        { 
            ull start_time = currentSeconds();
            seq_result = seq_sum(data);
            ull end_time = currentSeconds();
            seq_time += (end_time - start_time);
        }

        escape(&par_result);
        escape(&seq_result);

        if (par_result != seq_result) {
            std::cerr << std::fixed
                      << "Parallel result is not equal to sequential result:\n"
                      << "Parallel: " << par_result << "\n"
                      << "Sequential: " << seq_result << "\n";
            exit(-1);
        }

        if (cycle % CYCLE_STOP == 0) {
            std::cout << std::fixed
                      << "Step " << cycle / CYCLE_STOP << " of 10:\n"
                      << "Parallel time: " << par_time / cycle << " ns\n"
                      << "Sequential time: " << seq_time / cycle << " ns\n\n";
        }
    }

    std::cout << std::fixed
              << "Final results:\n"
              << "Parallel time: " << par_time / NUM_CYCLES << " ns\n"
              << "Sequential time: " << seq_time / NUM_CYCLES << " ns\n";
    return 0;
}
