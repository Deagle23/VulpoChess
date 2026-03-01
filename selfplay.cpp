#include "Search.hpp"
#include "chess.hpp"
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

std::string getGameTitle() {
    using namespace std::chrono;

    auto now = system_clock::now();
    std::time_t t = system_clock::to_time_t(now);

    std::tm tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << "Chess game at "
        << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S")
        << ".txt";

    return oss.str();
}


namespace fs = std::filesystem;

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

void writeGameToFolder(const std::string& folderName, const std::string& fileName, const std::vector<Move>& moves) {

    fs::path folderPath = folderName;

    fs::create_directories(folderPath);  // safe even if already exists

    fs::path filePath = folderPath / fileName;

    std::ofstream gameFile(filePath);

    if (!gameFile.is_open()) {
        std::cerr << "Failed to open file: " << filePath << "\n";
        return;
    }

    for (const Move& move : moves) {
        gameFile << uci::moveToUci(move) << "\n";
    }
}

int main() {

    Board board;

    Movelist moves;

    movegen::legalmoves(moves, board);

    GameResult x = board.isGameOver().second;

    std::vector<Move> game;

    while (x == GameResult::NONE) {
        Move move = findBestMove(board, 100);
        std::cout << uci::moveToSan(board, move) << "\n";
        board.makeMove(move);
        game.emplace_back(move);
        x = board.isGameOver().second;
    }

    printGameResult(board);

    writeGameToFolder("./games", getGameTitle(), game);


    return 0;

}