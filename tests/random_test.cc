#include <iostream>

#include "../generate_graph.h"

const u32 SEQ_SIZE = 30;

int main() {
    const RandomSequence R(SEQ_SIZE);
    for (u32 i = 0; i < SEQ_SIZE; ++i) {
        std::cout << R[i] << " ";
    }
    std::cout << "\n";

    return 0;
}