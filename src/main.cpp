#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include "chess/Fen.h"
#include "chess/Perft.h"
#include "chess/Notation.h"
#include "ui/GameLoop.h"

namespace {

// Usage: Chess.exe --perft <depth> [fen]
int runPerft(int argc, char** argv) {
    const int depth = std::atoi(argv[2]);
    const auto position = chess::parseFen(argc >= 4 ? argv[3] : chess::kStartingFen);
    if (depth < 1 || !position) {
        std::cerr << "Usage: Chess --perft <depth >= 1> [fen]\n";
        return 2;
    }
    const auto start = std::chrono::steady_clock::now();
    std::uint64_t total = 0;
    for (const auto& entry : chess::perftDivide(*position, depth)) {
        std::cout << chess::moveToString(entry.first) << ": " << entry.second << '\n';
        total += entry.second;
    }
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - start).count();
    std::cout << "\nNodes: " << total << "  (" << ms << " ms)\n";
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc >= 3 && std::string(argv[1]) == "--perft") return runPerft(argc, argv);
    ui::runHumanVsHuman(std::cin, std::cout, chess::Position::startingPosition());
    return 0;
}
