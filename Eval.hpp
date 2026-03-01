#pragma once
#define CHESS_NO_EXCEPTIONS
#include "chess.hpp"

using namespace chess;

const int PAWN_VALUE = 100;
const int KNIGHT_VALUE = 300;
const int BISHOP_VALUE = 350;
const int ROOK_VALUE = 500;
const int QUEEN_VALUE = 900;

constexpr int TEMPO_BONUS = 50;

const int PAWN_TABLE[64] = {
  0,   0,   0,   0,   0,   0,   0,   0,
  5,  10,  10, -20, -20,  10,  10,   5,
  5,  -5, -10,   0,   0, -10,  -5,   5,
  0,   0,   0,  20,  20,   0,   0,   0,
  5,   5,  10,  25,  25,  10,   5,   5,
 10,  10,  20,  30,  30,  20,  10,  10,
 50,  50,  50,  50,  50,  50,  50,  50,
  0,   0,   0,   0,   0,   0,   0,   0
};

const int KNIGHT_TABLE[64] = {
 -50,-40,-30,-30,-30,-30,-40,-50,
 -40,-20,  0,  0,  0,  0,-20,-40,
 -30,  0, 10, 15, 15, 10,  0,-30,
 -30,  5, 15, 20, 20, 15,  5,-30,
 -30,  0, 15, 20, 20, 15,  0,-30,
 -30,  5, 10, 15, 15, 10,  5,-30,
 -40,-20,  0,  5,  5,  0,-20,-40,
 -50,-40,-30,-30,-30,-30,-40,-50
};

const int BISHOP_TABLE[64] = {
 -20,-10,-10,-10,-10,-10,-10,-20,
 -10,  5,  0,  0,  0,  0,  5,-10,
 -10, 10, 10, 10, 10, 10, 10,-10,
 -10,  0, 10, 10, 10, 10,  0,-10,
 -10,  5,  5, 10, 10,  5,  5,-10,
 -10,  0,  5, 10, 10,  5,  0,-10,
 -10,  0,  0,  0,  0,  0,  0,-10,
 -20,-10,-10,-10,-10,-10,-10,-20
};

const int ROOK_TABLE[64] = {
   0,  0,  0,  5,  5,  0,  0,  0,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
  5, 10, 10, 10, 10, 10, 10,  5,
  0,  0,  0,  0,  0,  0,  0,  0
};

const int QUEEN_TABLE[64] = {
 -20,-10,-10, -5, -5,-10,-10,-20,
 -10,  0,  0,  0,  0,  0,  0,-10,
 -10,  0,  5,  5,  5,  5,  0,-10,
  -5,  0,  5,  5,  5,  5,  0, -5,
   0,  0,  5,  5,  5,  5,  0, -5,
 -10,  5,  5,  5,  5,  5,  0,-10,
 -10,  0,  5,  0,  0,  0,  0,-10,
 -20,-10,-10, -5, -5,-10,-10,-20
};

const int KING_TABLE[64] = {
 -30,-40,-40,-50,-50,-40,-40,-30,
 -30,-40,-40,-50,-50,-40,-40,-30,
 -30,-40,-40,-50,-50,-40,-40,-30,
 -30,-40,-40,-50,-50,-40,-40,-30,
 -20,-30,-30,-40,-40,-30,-30,-20,
 -10,-20,-20,-20,-20,-20,-20,-10,
  20, 20,  0,  0,  0,  0, 20, 20,
  20, 30, 10,  0,  0, 10, 30, 20
};

const int KING_ENDGAME_TABLE[64] = {
 -50,-40,-30,-20,-20,-30,-40,-50,
 -30,-20,-10,  0,  0,-10,-20,-30,
 -30,-10, 20, 30, 30, 20,-10,-30,
 -30,-10, 30, 40, 40, 30,-10,-30,
 -30,-10, 30, 40, 40, 30,-10,-30,
 -30,-10, 20, 30, 30, 20,-10,-30,
 -30,-30,  0,  0,  0,  0,-30,-30,
 -50,-30,-30,-30,-30,-30,-30,-50
};

int evalSquarePSQT(int sq, PieceType type, double egweight) {

    switch (type) {
        case PAWN:   return PAWN_TABLE[sq];
        case KNIGHT: return KNIGHT_TABLE[sq];
        case BISHOP: return BISHOP_TABLE[sq];
        case ROOK:   return ROOK_TABLE[sq];
        case QUEEN:  return QUEEN_TABLE[sq];
        case KING:   return (int)((1.0 - egweight) * KING_TABLE[sq] + egweight * KING_ENDGAME_TABLE[sq]);
        default:     return 0;
    }
}

inline int mirror(int sq) {
    return sq ^ 56;
}

int returnPSQT(const Board& board, PieceType type, double egweight) {
    int score = 0;
    Bitboard bb;
    bb = board.pieces(type, Color::WHITE);
    for (int i=0;i<2;i++) {
        while (bb) {
            int index = bb.pop();
            score += (i == 0) ? evalSquarePSQT(index, type, egweight) : -evalSquarePSQT(mirror(index), type, egweight);
        }
        bb = board.pieces(type, Color::BLACK);
    }

    return score;
}

int countControlledSquares(const Board& board, Color color) {
    return board.us(Color::WHITE).count();
}


int countMaterial(const chess::Board& board) {
    int pMaterial = 0;
    pMaterial += (board.pieces(PieceType::PAWN, Color::WHITE).count() + board.pieces(PieceType::PAWN, Color::BLACK).count()) * PAWN_VALUE;
    pMaterial += (board.pieces(PieceType::KNIGHT, Color::WHITE).count() + board.pieces(PieceType::KNIGHT, Color::BLACK).count()) * KNIGHT_VALUE;
    pMaterial += (board.pieces(PieceType::BISHOP, Color::WHITE).count() + board.pieces(PieceType::BISHOP, Color::BLACK).count()) * BISHOP_VALUE;
    pMaterial += (board.pieces(PieceType::ROOK, Color::WHITE).count() + board.pieces(PieceType::ROOK, Color::BLACK).count()) * ROOK_VALUE;
    pMaterial += (board.pieces(PieceType::QUEEN, Color::WHITE).count() + board.pieces(PieceType::QUEEN, Color::BLACK).count()) * QUEEN_VALUE;
    return pMaterial;
}

int getPieceValue(PieceType piece) {
    switch (piece) {
        case PAWN:   return PAWN_VALUE;
        case KNIGHT: return KNIGHT_VALUE;
        case BISHOP: return BISHOP_VALUE;
        case ROOK:   return ROOK_VALUE;
        case QUEEN:  return QUEEN_VALUE;
        default: return 0;
    }
}

int evalBoard(const chess::Board& board) {

    int score = 0;

    //Pure material
    int pMaterial = 0;
    pMaterial += (board.pieces(PieceType::PAWN, Color::WHITE).count() - board.pieces(PieceType::PAWN, Color::BLACK).count()) * PAWN_VALUE;
    pMaterial += (board.pieces(PieceType::KNIGHT, Color::WHITE).count() - board.pieces(PieceType::KNIGHT, Color::BLACK).count()) * KNIGHT_VALUE;
    pMaterial += (board.pieces(PieceType::BISHOP, Color::WHITE).count() - board.pieces(PieceType::BISHOP, Color::BLACK).count()) * BISHOP_VALUE;
    pMaterial += (board.pieces(PieceType::ROOK, Color::WHITE).count() - board.pieces(PieceType::ROOK, Color::BLACK).count()) * ROOK_VALUE;
    pMaterial += (board.pieces(PieceType::QUEEN, Color::WHITE).count() - board.pieces(PieceType::QUEEN, Color::BLACK).count()) * QUEEN_VALUE;

    //Increases to 1 as the game goes on
    double endgameWeight = ((float) 8000 - (float) countMaterial(board)) / 8000;
    if (endgameWeight < 0.0) endgameWeight = 0.0;
    if (endgameWeight > 1.0) endgameWeight = 1.0;
    

    score += pMaterial;

    //PSQT stuff

    score += returnPSQT(board, PieceType::PAWN, endgameWeight);
    score += returnPSQT(board, PieceType::KNIGHT, endgameWeight);
    score += returnPSQT(board, PieceType::BISHOP, endgameWeight);
    score += returnPSQT(board, PieceType::ROOK, endgameWeight);
    score += returnPSQT(board, PieceType::QUEEN, endgameWeight);
    score += returnPSQT(board, PieceType::KING, endgameWeight);

    int mobility = countControlledSquares(board, Color::WHITE) - countControlledSquares(board, Color::BLACK);

    score += mobility;

    //Tempo
    score += TEMPO_BONUS;


    return board.sideToMove() == Color::WHITE ? score : -score;
}