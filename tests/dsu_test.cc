#include <algorithm>
#include <iostream>
#include <omp.h>
#include <pair>
#include <random>
#include <vector>

#include "../defs.h"
#include "../dsu.h"

struct SequentialDSU {
    u32 size;
    std::vector<u32> parent;
    std::vector<u32> rank;

    SequentialDSU(u32 size) : size(size), parent(size), rank(size) {
        for (u32 i = 0; i < size; ++i) parent[i] = i;
    }

    u32 find_root(u32 id) {
        while (id != parent[id]) {
            id = parent[id];
        }

        return id;
    }

    bool same_set(u32 id1, u32 id2) {
        return find_root(id1) == find_root(id2);
    }

    void unite(u32 id1, u32 id2) {
        id1 = find_root(id1);
        id2 = find_root(id2);

        if (rank[id1] < rank[id2]) std::swap(id1, id2);

        parent[id2] = id1;
        if (rank[id1] == rank[id2]) ++rank[id1];
    }
};

std::random_device rd;
std::mt19937 gen(rd());

u32 randint(u32 l, u32 r) {
    return std::uniform_int_distribution<u32>(l, r)(gen);
}

const u32 NUM_STEPS = 100;
const u32 SMALL_NUM_STEPS = 100;
const u32 MAX_SIZE = 100'000'000;
const u32 SMALL_SIZE = 50;

void dump_data(const u32 size,
               const std::vector<std::pair<u32, u32>>& queries,
               SequentialDSU correct,
               DSU incorrect,
               u32 a,
               u32 b) {
    std::cerr << "Component mismatch:\n"
                              << "Size: " << size << "\n"
                              << "Queries:\n";
    for (auto p : queries) {
        std::cerr << p.first << " " << p.second << "\n";
    }
    std::cerr << "Correct parents:\n";
    for (u32 i = 0; i < size; ++i) {
        std::cerr << correct.find_root(i) << " ";
    }
    std::cerr << "\nIncorrect parents:\n";
    for (u32 i = 0; i < size; ++i) {
        std::cerr << incorrect.find_root(i) << " ";
    }
    std::cerr << "\nComponents of " << a << " " << b << "\n";
}

int main() {
    std::cout << "Checking correctness:\n";
    std::cout << "Checking small sizes:\n";
    for (u32 size = 3; size <= SMALL_SIZE; ++size) {
        std::cout << "Size: " << size << "\n";
        for (u32 step = 0; step < SMALL_NUM_STEPS; ++step) {
            std::cout << "Step: " << step + 1 << "\n";

            DSU to_check(size);
            SequentialDSU correct(size);

            std::vector<std::pair<u32, u32>> queries(randint(1, size * 5));
            for (auto& p : queries) {
                u32 l = randint(0, size - 1);
                u32 r = randint(0, size - 1);
                p.first = std::min(l, r);
                p.second = std::max(l, r);
            }

            for (auto p : queries) {
                correct.unite(p.first, p.second);
            }

            std::cout << "Parallel started working\n";
            #pragma omp parallel for
            for (auto p : queries) {
                to_check.unite(p.first, p.second);
            }
            std::cout << "Parallel finished working\n";

            for (u32 i = 0; i < size; ++i) {
                for (u32 j = i; j < size; ++j) {
                    if (correct.same_set(i, j) != to_check.same_set(i, j)) {
                        dump_data(size, queries, correct, to_check, i, j);
                        exit(-1);
                    }
                }
            }
            std::cout << "OK\n";
        }
    }
    std::cout << "OK\n";
    std::cout << "Checking on random queries:\n";
    for (u32 i = 0; i < NUM_STEPS; ++i) {
        u32 size = randint(MAX_SIZE / 100, MAX_SIZE);
        DSU to_check(size);
        SequentialDSU correct(size);

        std::vector<std::pair<u32, u32>> queries(randint(1, size * 1.5));  // 1.5 x size operation at max
        for (auto& p : queries) {
            u32 l = randint(0, size - 1);
            u32 r = randint(0, size - 1);
            p.first = std::min(l, r);
            p.second = std::max(l, r);
        }

        for (auto p : queries) {
            correct.unite(p.first, p.second);
        }

        #pragma omp parallel for
        for (auto p : queries) {
            to_check.unite(p.first, p.second);
        }

        for (u32 i = 0; i < size; ++i) {
            for (u32 j = i; j < size; ++j) {
                if (correct.same_set(i, j) != to_check.same_set(i, j)) {
                    dump_data(size, queries, correct, to_check, i, j);
                    exit(-1);
                }
            }
        }
    }
    std::cout << "OK\n";
    std::cout << "Performance (TODO)\n";
    return 0;
}