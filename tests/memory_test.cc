#include <iostream>
#include <vector>

#include "../benchmark.h"
#include "../timer.h"

struct T {
    int a, b, c;

    T() {}
};

const size_t ITER = 100'000;
const size_t SIZE = 10'000'000;

int main() {
    {
        double start = currentSeconds();
        for (size_t i = 0; i < ITER; ++i) {
            T* arr = new T[SIZE];
            delete[] arr;
        }
        double finish = currentSeconds();
        std::cout << std::fixed << "default " << (finish - start) << "ns\n";
    }

    {
        double start = currentSeconds();
        for (size_t i = 0; i < ITER; ++i) {
            T* arr = static_cast<T*>(operator new[] (SIZE * sizeof(T)));
            delete[] arr;
        }
        double finish = currentSeconds();
        std::cout << std::fixed << "uninitialized " << (finish - start) << "ns\n";
    }

    {
        double start = currentSeconds();
        for (size_t i = 0; i < ITER; ++i) {
            std::vector<T> arr(SIZE);
        }
        double finish = currentSeconds();
        std::cout << std::fixed << "vector " << (finish - start) << "ns\n";
    }
}