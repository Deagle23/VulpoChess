#include <ctime>
#define CHESS_NO_EXCEPTIONS

#include "chess.hpp"
#include <iostream>
#include "Eval.hpp"
#include "TT.hpp"
// #include <queue>
// #include <map>
#include <utility>
#include <time.h>
#include <chrono>

using namespace chess;

constexpr int INF = 1'000'000'000;
constexpr int ROOT_BONUS = 2'000'0000;

int SEE(Board& board, Move move) {
    int victim = getPieceValue(board.at(move.to()).type());
    int attacker = getPieceValue(board.at(move.from()).type());
    return victim * 10 - attacker;
}

int scoreMove(Board& board, Move move) {
    //SEE (REPLACE SOON)
    int score = 0;
    if (board.isCapture(move)) {
        score = SEE(board, move);
    }
    return score;
}

void scoreMoves(Board& board, Movelist& moves, int scores[256], bool root=false, Move bestMove=chess::Move::NULL_MOVE) {
    for (int i=0;i<moves.size();i++) {
        scores[i] = scoreMove(board, moves.at(i));
        if (root && moves.at(i) == bestMove) {
            scores[i] += ROOT_BONUS;
        }
    }
}



int negamax(Board& board, int depth, int alpha, int beta) {

    if (depth == 0) {
        return evalBoard(board);
    }
    
    Movelist moves;
    movegen::legalmoves(moves, board);
    int scores[256];
    scoreMoves(board, moves, scores);

    if (moves.empty()) {
        if (board.inCheck()) {
            return -INF + 1;
        } else {
            return 0;
        }
    }

    for (int i=0;i<moves.size();i++) {
        int bestIndex = i;
        //Find the actual best index
        for (int j=i+1;j<moves.size();j++) {
            if (scores[bestIndex] < scores[j]) {
                bestIndex = j;
            }
        }

        //Swap it into the front
        std::swap(moves[bestIndex], moves[i]);
        std::swap(scores[bestIndex], scores[i]);

        board.makeMove(moves[i]);
        int score = -negamax(board, depth - 1, -beta, -alpha);
        board.unmakeMove(moves[i]);

        if (score >= beta) {
            return beta;
        }
        if (score > alpha) {
            alpha = score;
        }
    }


    return alpha;
}

Move findBestMove(Board& board, int timeAllocated=10'000, bool printEval=false, int depth=1) {

    
    Movelist moves;
    movegen::legalmoves(moves, board);

    Move bestMove;
    int bestScore = -INF;

    using Clock = std::chrono::steady_clock;

    Clock::time_point start = Clock::now();
    auto now = Clock::now();

    bool over = false;

    Move prevBestMove;
    int prevEval = 0;

    while (!over) {
        bestScore = -INF;

        //Order moves
        int scores[256];

        (depth > 1) ? scoreMoves(board, moves, scores, true, bestMove) : scoreMoves(board, moves, scores);

        for (int i=0;i<moves.size();i++) {
            int bestIndex = i;
            int bestMoveScore = scores[i];

            for (int j=i+1;j<moves.size();j++) {
                if (scores[j] > bestMoveScore) {
                    bestMoveScore = scores[j];
                    bestIndex = j;
                }
            }

            //Swap it to the front
            std::swap(moves[bestIndex], moves[i]);
            std::swap(scores[bestIndex], scores[i]);

            Move move = moves[i];

            board.makeMove(move);
            int score = -negamax(board, depth - 1, -INF, INF);
            board.unmakeMove(move);

            now = Clock::now();

            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }

            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start).count();

            if (ms >= timeAllocated && depth > 1) {
                over = true;
                break;
            }
        }
        depth += 1;
        prevBestMove = bestMove;
        prevEval = bestScore;
    }

    if (printEval) {std::cout << "\nEval: " << prevEval << "\nDepth: " << depth << "\n";}


    return prevBestMove;
}