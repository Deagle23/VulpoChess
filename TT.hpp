#pragma once

#include "chess.hpp"
#include <vector>
#include <cstdint>

using namespace chess;

enum TTFlag : uint8_t {
    EXACT,
    LOWERBOUND,
    UPPERBOUND
};

struct TTEntry {
    uint64_t key = 0;
    int depth = -1;
    int score = 0;
    TTFlag flag = EXACT;
    Move bestMove{};
};

struct TT {

    std::vector<TTEntry> table;
    size_t mask = 0;

    TT(size_t mb = 64) {
        resize(mb);
    }

    void resize(size_t mb) {

        size_t bytes = mb * 1024ULL * 1024ULL;
        size_t count = bytes / sizeof(TTEntry);

        //Round down to nearest power of two
        count = 1ULL << (63 - __builtin_clzll(count));

        table.resize(count);
        mask = count - 1;
    }

    void clear() {
        for (auto& e : table) {
            e.key = 0;
            e.depth = -1;
        }
    }

    TTEntry* probe(uint64_t key) {
        
        //Basically rough modulo by the size of the TT
        TTEntry& entry = table[key & mask];

        if (entry.key == key)
            return &entry;

        return nullptr;
    }

    void store(uint64_t key,
               int depth,
               int score,
               TTFlag flag,
               Move bestMove)
    {
        TTEntry& entry = table[key & mask];

        if (bestMove == Move::NULL_MOVE) {return;}

        // replace if empty or deeper search
        if (entry.key != key || depth >= entry.depth) {

            entry.key = key;
            entry.depth = depth;
            entry.score = score;
            entry.flag = flag;
            entry.bestMove = bestMove;
        }
    }
};