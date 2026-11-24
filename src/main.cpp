#include <chrono>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include "chess/Fen.h"
#include "chess/Notation.h"
#include "chess/Perft.h"
#include "engine/Search.h"
#include "ui/GameLoop.h"
#include "ui/PlayerSetup.h"

namespace {

// A computer-only game has no draw rules to end it yet, so cap its length.
constexpr int kComputerOnlyPlyLimit = 1000;

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

// Usage: Chess.exe --search <depth> [fen]   (one search; prints statistics)
int runSearch(int argc, char** argv) {
    const int depth = std::atoi(argv[2]);
    const auto position = chess::parseFen(argc >= 4 ? argv[3] : chess::kStartingFen);
    if (depth < 1 || !position) {
        std::cerr << "Usage: Chess --search <depth >= 1> [fen]\n";
        return 2;
    }
    const engine::SearchResult r = engine::findBestMove(*position, depth);
    std::cout << "Best move: " << chess::moveToString(r.bestMove) << "\nScore: " << r.score
              << " (side to move)\nDepth: " << r.stats.depth << "\nNodes: " << r.stats.nodes
              << "\nTime: " << r.stats.seconds << " s\n";
    return 0;
}

// Asks for the player type of one side, plus a search depth if needed. False at end of input.
bool configureSide(const char* name, ui::PlayerKind& kind, int& depth) {
    const auto k = ui::promptPlayerKind(std::cin, std::cout, name);
    if (!k) return false;
    kind = *k;
    depth = ui::kDefaultSearchDepth;
    if (kind == ui::PlayerKind::SearchComputer) {
        const auto d = ui::promptSearchDepth(std::cin, std::cout);
        if (!d) return false;
        depth = *d;
    }
    return true;
}

}  // namespace

// Usage: Chess.exe [--seed <n>]      (--seed makes random computer players reproducible)
//        Chess.exe --perft <depth> [fen]
//        Chess.exe --search <depth> [fen]
int main(int argc, char** argv) {
    if (argc >= 3 && std::string(argv[1]) == "--perft") return runPerft(argc, argv);
    if (argc >= 3 && std::string(argv[1]) == "--search") return runSearch(argc, argv);

    std::optional<std::uint32_t> seed;
    if (argc >= 3 && std::string(argv[1]) == "--seed")
        seed = static_cast<std::uint32_t>(std::strtoul(argv[2], nullptr, 10));

    ui::PlayerKind whiteKind = ui::PlayerKind::Human, blackKind = ui::PlayerKind::Human;
    int whiteDepth = 0, blackDepth = 0;
    if (!configureSide("White", whiteKind, whiteDepth)) return 0;
    if (!configureSide("Black", blackKind, blackDepth)) return 0;

    const auto whitePlayer = ui::makePlayer(whiteKind, std::cin, std::cout, seed, whiteDepth);
    const auto blackPlayer = ui::makePlayer(blackKind, std::cin, std::cout,
                                            seed ? std::optional<std::uint32_t>(*seed + 1) : std::nullopt,
                                            blackDepth);

    ui::GameOptions options;
    if (whiteKind != ui::PlayerKind::Human && blackKind != ui::PlayerKind::Human)
        options.maxPlies = kComputerOnlyPlyLimit;

    ui::runGame(*whitePlayer, *blackPlayer, std::cout, chess::Position::startingPosition(), options);
    return 0;
}
