#include <chrono>
#include "Eval.hpp"
#include "Search.hpp"
#include "chess.hpp"
#include <fstream>
#include <string>
#include <iostream>

using Clock = std::chrono::steady_clock;

__attribute__((noinline))
int timeTakenDepth(Board& board, int depth)
{
    Clock::time_point start = Clock::now();

    Move move = findBestMoveDepth(board, depth);
    if (move == Move::NO_MOVE) std::abort();

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        Clock::now() - start).count();

    return ms;
}

int main()
{
    float average = 0.0f;
    int linesCounted = 0;
    std::string line;

    std::ifstream MyReadFile("positions.txt");

    if (!MyReadFile) {
        std::cerr << "Failed to open file\n";
        return 1;
    }

    int depth = 7;

    while (std::getline(MyReadFile, line))
    {
        Board board(line);

        int timeTaken = timeTakenDepth(board, depth);

        linesCounted++;
        average += (timeTaken - average) / linesCounted;
    }

    std::cout << "Average time taken to search to depth "
              << depth << ": "
              << average << " ms\n";
}