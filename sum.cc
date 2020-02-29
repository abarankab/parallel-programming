#include <iostream>
#include <stdlib.h>
#include <vector>
#include <omp.h>
#include <random>

#include "benchmark.h"
#include "timer.h"

std::vector<double> init_data() {
    std::vector<double> data(rand() % 10'000'000);
    for (double& x : data) {
        x = rand();
    }
    return data;
}

double par_sum(std::vector<double>& v) {
    double res = 0;
    for (auto x : v) res += x;
    return res;
}

double seq_sum(std::vector<double>& v) {
    double res = 0;
    for (auto x : v) res += x;
    return res;
}

int main() {
    const size_t NUM_CYCLES = 1'000;
    const size_t CYCLE_STOP = NUM_CYCLES / 10;

    double par_time = 0;
    double seq_time = 0;

    std::cout << "Start\n";

    for (size_t cycle = 1; cycle <= NUM_CYCLES; ++cycle) {
        std::vector<double> data = init_data();
        escape(&data);

        {
            double start_time = currentSeconds();
            double result = par_sum(data);
            double end_time = currentSeconds();
            escape(&result);
            par_time += (end_time - start_time);
        }

        { 
            double start_time = currentSeconds();
            double result = seq_sum(data);
            double end_time = currentSeconds();
            escape(&result);
            seq_time += (end_time - start_time);
        }

        if (cycle % CYCLE_STOP == 0) {
            std::cout << std::fixed
                      << "Step " << cycle / CYCLE_STOP << " of 10:\n"
                      << "Parallel time: " << par_time / cycle << " ns\n"
                      << "Sequential time: " << seq_time / cycle << " ns\n";
        }
    }

    std::cout << std::fixed
              << "Final results:\n"
              << "Parallel time: " << par_time / NUM_CYCLES << " ns\n"
              << "Sequential time: " << seq_time / NUM_CYCLES << " ns\n";
    return 0;
}
