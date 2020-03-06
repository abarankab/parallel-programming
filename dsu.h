#ifndef __DSU_H
#define __DSU_H

#include <atomic>
#include <omp.h>
#include <stdexcept>

#include "defs.h"

/**
 * This implementation was inspired by this repo
 * https://github.com/wjakob/dset/blob/master/dset.h
 * and this paper
 * http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.56.8354&rep=rep1&type=pdf
 * It uses both rank and path heuristics in parallel
 * which should allow for O(\alpha S) time on both find_root and unite operations
 * 
 * Data is stored in unsigned 64 bit integers
 * The first 32 bits encode node parent
 * The last 32 bits encode node rank
 * This allows for easier compare and swap because you cannot perform it on classes
 * 
 * E.g. if X is the stored value:
 * X & 0x00000000FFFFFFFF <- parent
 * X & 0xFFFFFFFF00000000 <- rank
 * 
 * To decode these values from u64 one should use get_parent and get_rank
 * 
 * I also check if id is within range, this slows the code down a little bit
 * but should save you some time debugging
 */
struct DSU {
    u32 size;

    atomic_u64* data;
    const u32 BINARY_BUCKET_SIZE = 32;
    const u64 RANK_MASK = 0xFFFFFFFF00000000ULL;

    DSU(u32 size) : size(size) {
        data = static_cast<atomic_u64*>(operator new[] (size * sizeof(atomic_u64)));

        #pragma omp parallel for shared(data)
        for (u32 i = 0; i < size; ++i) data[i] = i;
    }

    ~DSU() {
        delete[] data;
    }

    void check_out_of_range(u32 id) {
        if (id >= size) {
            throw std::out_of_range("Node id out of range");
        }
    }

    u32 get_parent(u32 id) const {
        return static_cast<u32>(data[id]);
    }

    u32 get_rank(u32 id) const {
        return static_cast<u32>(data[id] >> BINARY_BUCKET_SIZE);
    }

    /**
     * On each step we modify encoded_value,
     * keeping its rank and changing its parent to grandparent
     * 
     * Then we try to apply path heuristic using CAS
     * 
     * The loop breaks when a node's parent is equal to itself
     * E.g. when we find the root
     */
    u32 find_root(u32 id) {
        check_out_of_range(id);

        while (id != get_parent(id)) {
            u64 encoded_value = data[id];
            u32 grandparent = get_parent(static_cast<u32>(encoded_value));
            u64 new_value = (encoded_value & RANK_MASK) | grandparent;

            /* Path heuristic */
            if (encoded_value != new_value) {
                data[id].compare_exchange_weak(encoded_value, new_value);
            }

            id = grandparent;
        }

        return id;
    }

    /**
     * We try to check if two nodes are in the same set
     * by checking if their roots are the same
     * 
     * Since it is a parallel structure, node roots may change during runtime
     * In order to account for this we do a while loop and repeat if
     * our current node is no longer the root of its set
     */
    bool same_set(u32 id1, u32 id2) {
        check_out_of_range(id1);
        check_out_of_range(id2);

        while (true) {
            id1 = find_root(id1);
            id2 = find_root(id2);

            if (id1 == id2) {
                return true;
            } else if (get_parent(id1) == id1) {
                return false;
            }
        }
    }

    /**
     * We try to hang the smaller component onto the bigger one
     * 
     * Since it is a parallel structure, node roots may change during runtime
     * In order to account for this we do a while loop and repeat if
     * the smaller node was updated e.g. when CAS failed
     */
    void unite(u32 id1, u32 id2) {
        check_out_of_range(id1);
        check_out_of_range(id2);

        while (true) {
            id1 = find_root(id1);
            id2 = find_root(id2);

            /* Nodes are already in the same set */
            if (id1 == id2) return;

            u32 rank1 = get_rank(id1);
            u32 rank2 = get_rank(id2);

            /* Hanging the smaller set to the bigger one, rank heuristic */
            if (rank1 < rank2) {
                std::swap(rank1, rank2);
                std::swap(id1, id2);
            }

            u64 old_value = (static_cast<u64>(rank2) << BINARY_BUCKET_SIZE) | id2;
            u64 new_value = (static_cast<u64>(rank2) << BINARY_BUCKET_SIZE) | id1;

            /* If CAS fails we need to repeat the same step once again */
            if (!data[id2].compare_exchange_strong(old_value, new_value)) continue;

            /* Updating rank */
            if (rank1 == rank2) {
                old_value = (static_cast<u64>(rank1) << BINARY_BUCKET_SIZE) | rank1;
                new_value = (static_cast<u64>(rank1 + 1) << BINARY_BUCKET_SIZE) | rank1;

                data[id1].compare_exchange_weak(old_value, new_value);
            }

            break;
        }
    }
};

#endif
