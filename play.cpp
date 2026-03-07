#define CHESS_NO_EXCEPTIONS

#include "chess.hpp"
#include "Search.hpp"
#include <iostream>
#include <string>
#include <chrono>

using namespace chess;

bool applyUserMove(Board& board, const std::string& input) {

    Movelist moves;
    movegen::legalmoves(moves, board);

    for (const auto& move : moves) {
        if (uci::moveToUci(move) == input) {
            board.makeMove(move);
            return true;
        }
    }

    return false;
}

void printGameResult(Board& board) {

    Movelist moves;
    movegen::legalmoves(moves, board);

    if (!moves.empty())
        return;

    if (board.inCheck()) {
        if (board.sideToMove() == Color::WHITE)
            std::cout << "Checkmate! Black wins.\n";
        else
            std::cout << "Checkmate! White wins.\n";
    } else {
        std::cout << "Stalemate!\n";
    }
}

int main() {

    Board board;

    int depth = 6;

    std::cout << "Simple Console Chess Engine\n";
    std::cout << "Enter moves in UCI format (e2e4, g1f3, e7e8q)\n\n";

    while (true) {

        std::cout << board << "\n";

        printGameResult(board);

        Movelist moves;
        movegen::legalmoves(moves, board);
        if (moves.empty())
            break;

        if (board.sideToMove() == Color::WHITE) {

            std::string input;
            std::cout << "Your move: ";
            std::cin >> input;

            if (!applyUserMove(board, input)) {
                std::cout << "Illegal move. Try again.\n";
                continue;
            }

        } else {

            std::cout << "Engine thinking...\n";

            using Clock = std::chrono::steady_clock;

            auto start = Clock::now();

            Move engineMove = findBestMove(board, 1500, true);

            auto end = Clock::now();

            auto elapsedMs =
                std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            std::cout << "Engine plays: "
                    << uci::moveToUci(engineMove)
                    << "\n";

            std::cout << "Time elapsed: "
                    << elapsedMs << " ms ("
                    << elapsedMs / 1000.0 << " s)\n";

            board.makeMove(engineMove);
        }
    }

    return 0;
}