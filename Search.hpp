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
constexpr int ROOT_BONUS = 100'000;
constexpr int CONTEMPT = 20;
constexpr int KILLER1_BONUS = 90'000;
constexpr int KILLER2_BONUS = 80'000;

Move killers[30][2] {};

int SEE(Board& board, Move move) {
    int victim = getPieceValue(board.at(move.to()).type());
    int attacker = getPieceValue(board.at(move.from()).type());
    return victim * 10 - attacker;
}

int scoreMove(Board& board, Move move, int ply=0) {
    //SEE (REPLACE SOON)
    int score = 0;
    if (board.isCapture(move)) {
        score = SEE(board, move);
    }
    if (ply != 0) {
        if (killers[ply][0] == move)
            score += KILLER1_BONUS;
        if (killers[ply][1] == move)
            score += KILLER2_BONUS;
    }
    return score;
}

void scoreMoves(Board& board, Movelist& moves, int scores[256], bool root=false, int ply=0,Move bestMove=chess::Move::NULL_MOVE) {
    for (int i=0;i<moves.size();i++) {
        scores[i] = scoreMove(board, moves.at(i), ply);
        if (root && moves.at(i) == bestMove) {
            scores[i] += ROOT_BONUS;
        }
    }
}

int qSearch(Board& board, int alpha=-INF, int beta=INF) {
    //A stand pat
    int bestScore = evalBoard(board);

    if (bestScore >= beta) {
        return beta;
    }
    if (bestScore > alpha) {
        alpha = bestScore;
    }

    Movelist captureMoves;

    movegen::legalmoves(captureMoves, board);

    if (captureMoves.empty()) {
        if (board.inCheck()) {
            return -INF + 1;
        } else {
            return bestScore;
        }
    }
    else if (board.halfMoveClock() >= 100 || board.isHalfMoveDraw() || board.isInsufficientMaterial() || board.isRepetition(1)) {
        //Prefers to not draw
        return (board.sideToMove() == Color::WHITE ? -CONTEMPT : CONTEMPT);
    }
    
    int score = -INF;

    for (auto move : captureMoves) {
        if (!board.isCapture(move)) {continue;}
        board.makeMove(move);
        score = -qSearch(board, -beta, -alpha);
        board.unmakeMove(move);

        if (score >= beta) {
            return beta;
        }
        if (score > bestScore) {
            bestScore = score;
        }
        if (score > alpha) {
            alpha = score;
        }
        
    }
    return alpha;

}



int negamax(Board& board, int depth, int alpha=-INF, int beta=INF, int ply=1) {

    if (depth == 0) {
        return qSearch(board, alpha, beta);
    }
    
    Movelist moves;
    movegen::legalmoves(moves, board);
    int scores[256];
    scoreMoves(board, moves, scores, false, ply - 1);

    if (moves.empty()) {
        if (board.inCheck()) {
            return -INF + 1;
        } else {
            //Make it doesn't like draws
            return (board.sideToMove() == Color::WHITE ? -CONTEMPT : CONTEMPT);
        }
    }

    else if (board.halfMoveClock() >= 100 || board.isHalfMoveDraw() || board.isInsufficientMaterial() || board.isRepetition(1)) {
        return (board.sideToMove() == Color::WHITE ? -CONTEMPT : CONTEMPT);
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
            if (!board.isCapture(moves[i])) {
                killers[ply - 1][1] = killers[ply - 1][0];
                killers[ply - 1][0] = moves[i];
            }
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

        (depth > 1) ? scoreMoves(board, moves, scores, true, 0, bestMove) : scoreMoves(board, moves, scores);

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
            int score = -negamax(board, depth - 1, -INF, INF, 1);
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