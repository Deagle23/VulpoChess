#include <ctime>
#define CHESS_NO_EXCEPTIONS

#include "chess.hpp"
#include <algorithm>
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
constexpr int TT_BONUS = 95'000;
constexpr int KILLER1_BONUS = 50'000;
constexpr int KILLER2_BONUS = 40'000;
constexpr int MAX_HISTORY = 30'000;
constexpr int HISTORY_BONUS = 5'000;

Move killers[64][2] {};
int history[2][64][64] {};

TT table(64);

void updateHistory(Color color, Move move, int bonus) {
    int side = (color == Color::WHITE) ? 0 : 1;
    int* hsPointer = &history[side][move.from().index()][move.to().index()];
    *hsPointer += bonus - (*hsPointer * abs(bonus)) / MAX_HISTORY;
    *hsPointer = std::clamp(*hsPointer, -MAX_HISTORY, MAX_HISTORY);
}

int SEE(Board& board, Move move) {
    int victim = getPieceValue(board.at(move.to()).type());
    int attacker = getPieceValue(board.at(move.from()).type());
    return victim * 10 - attacker;
}

int scoreMove(Board& board, Move move, Move TTmove, int ply=0) {
    int score = 0;

    if (TTmove == move) {
        score += TT_BONUS;
    }

    if (board.isCapture(move)) {
        score += SEE(board, move);
    }
    else {
        score += history[(board.sideToMove() == Color::WHITE) ? 0 : 1][move.from().index()][move.to().index()];
    if (ply != 0) {
        if (killers[ply][0] == move)
            score += KILLER1_BONUS;
        if (killers[ply][1] == move)
            score += KILLER2_BONUS;
    }
    }
    return score;
}

void scoreMoves(Board& board, Movelist& moves, int scores[256], bool root=false, int ply=0,Move bestMove=chess::Move::NULL_MOVE) {
    TTEntry* entry = table.probe(board.hash());
    Move TTmove = (entry) ? entry->bestMove : Move::NULL_MOVE;
    for (int i=0;i<moves.size();i++) {
        scores[i] = scoreMove(board, moves.at(i), TTmove, ply);
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
        return -CONTEMPT;
    }
    
    int score = -INF;
    int currentPawnCache = pawnCache;

    for (auto move : captureMoves) {
        if (!board.isCapture(move)) {continue;}
        board.makeMove(move);
        if (board.at(move.to()).type() == PieceType::PAWN) {
            evalPawns(board);
        }
        score = -qSearch(board, -beta, -alpha);
        board.unmakeMove(move);
        pawnCache = currentPawnCache;

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

    if (depth <= 0) {
        return qSearch(board, alpha, beta);
    }

    int hash = board.hash();
    TTEntry* entry = table.probe(hash);
    if (entry && entry->depth >= depth && entry->key == hash)  {
        switch (entry->flag) {
            case EXACT: return entry->score;
            case UPPERBOUND: beta = std::min(entry->score, beta); break;
            case LOWERBOUND: alpha = std::max(entry->score, alpha); break;
        }
    }
    
    Movelist moves;
    movegen::legalmoves(moves, board);
    int scores[256];
    scoreMoves(board, moves, scores, false, ply);

    Move bestMove = Move::NULL_MOVE;

    int ogAlpha = alpha;

    if (alpha >= beta) {
        return beta;
    }

    if (moves.empty()) {
        if (board.inCheck()) {
            return -INF + 1;
        } else {
            //Make it doesn't like draws
            return -CONTEMPT;
        }
    }

    else if (board.halfMoveClock() >= 100 || board.isHalfMoveDraw() || board.isInsufficientMaterial() || board.isRepetition(1)) {
        return -CONTEMPT;
    }

    int currentPawnCache = pawnCache;

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
        if (board.at(moves[i].to()).type() == PieceType::PAWN) {
            evalPawns(board);
        }
        //If it's a quiet move
        int reduction = 0;
        bool reduced = false;
        if ((i > 4) && (ogAlpha == alpha) && depth > 2) {
            reduction = (i / moves.size()) * 3;
            reduced = true;
        }
        int score = -negamax(board, depth - 1 - reduction, -beta, -alpha, ply++);
        board.unmakeMove(moves[i]);
        pawnCache = currentPawnCache;


        //Too good––opponent won't allow it, beta cutoff
        if (score >= beta) {
            if (!board.isCapture(moves[i])) {
                //Add to killer
                killers[ply][1] = killers[ply][0];
                killers[ply][0] = moves[i];
                //Add to history
                int bonus = depth * depth;
                updateHistory(board.sideToMove(), moves[i], bonus);
            }
            table.store(hash, depth, beta, LOWERBOUND, moves[i]);
            return beta;
        }
        if (score > alpha) {
            alpha = score;
            bestMove = moves[i];
            if (reduced) {
                //Search it at a greater depth than before
                board.makeMove(moves[i]);
                if (board.at(moves[i].to()).type() == PieceType::PAWN) {evalPawns(board);}
                score = -negamax(board, depth - 1, -beta, -alpha, ply++);
                board.unmakeMove(moves[i]);
                pawnCache = currentPawnCache;
                if (score >= beta) {
                    if (!board.isCapture(moves[i])) {
                        //Add to killer
                        killers[ply][1] = killers[ply][0];
                        killers[ply][0] = moves[i];
                        //Add to history
                        int bonus = depth * depth;
                        updateHistory(board.sideToMove(), moves[i], bonus);
                    }
                    table.store(hash, depth, beta, LOWERBOUND, moves[i]);
                    return beta;
                }
                if (score > alpha) {alpha = score; bestMove = moves[i];}
            }
        }
    }

    table.store(hash, depth, alpha, (ogAlpha <= alpha) ? UPPERBOUND : EXACT, bestMove);

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

    int aWindow, bWindow;
    aWindow = bWindow = INF;

    while (!over) {
        bestScore = -INF;
        int lb = prevEval - aWindow;
        int ub = prevEval + bWindow;

        //Order moves
        int scores[256];

        //Used for sort after
        int evalScores[256];

        (depth > 1) ? scoreMoves(board, moves, scores, true, 0, prevBestMove) : scoreMoves(board, moves, scores);

        while (true) {
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
            if (!(lb < bestScore && bestScore < ub)) {
                if (bestScore <= lb) {aWindow *= 2;}
                if (bestScore >= ub) {bWindow *= 2;}
                lb = prevEval - aWindow;
                ub = prevEval + bWindow;
                continue;
            } 
            break;
        }
        depth += 1;
        prevBestMove = bestMove;
        prevEval = bestScore;
        aWindow = bWindow = 25;
    }



    if (printEval) {std::cout << "\nEval: " << prevEval * (1 - 2 * (board.sideToMove() == Color::BLACK)) << "\nDepth: " << depth << "\n";}


    return prevBestMove;
}

