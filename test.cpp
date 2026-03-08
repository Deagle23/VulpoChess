#include "Eval.hpp"


void foo() {
    uint8_t pawns = 8;
    int rank = 1;
    while (((pawns & 1) != 1) && (pawns != 0)) {
        pawns = pawns << 1;
        rank += 1;
    }
    print(rank, 2);
}



int main() {
    foo();

    Board board("8/8/2p2k2/8/2P5/5K2/8/8 w - - 0 1");
    evalPawns(board);
    std::cout << pawnCache << '\n'; 
}