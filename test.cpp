#include "Eval.hpp"


void foo() {
}



int main() {

    Board board("8/8/2p2k2/8/2P5/2P2K2/8/8 w - - 0 1");
    evalPawns(board);
    std::cout << pawnCache << '\n'; 
}