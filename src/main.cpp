#include <chrono>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include "chess/Fen.h"
#include "chess/Notation.h"
#include "chess/Perft.h"
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

}  // namespace

// Usage: Chess.exe [--seed <n>]      (--seed makes computer players reproducible)
//        Chess.exe --perft <depth> [fen]
int main(int argc, char** argv) {
    if (argc >= 3 && std::string(argv[1]) == "--perft") return runPerft(argc, argv);

    std::optional<std::uint32_t> seed;
    if (argc >= 3 && std::string(argv[1]) == "--seed")
        seed = static_cast<std::uint32_t>(std::strtoul(argv[2], nullptr, 10));

    const auto whiteKind = ui::promptPlayerKind(std::cin, std::cout, "White");
    if (!whiteKind) return 0;
    const auto blackKind = ui::promptPlayerKind(std::cin, std::cout, "Black");
    if (!blackKind) return 0;

    const auto whitePlayer = ui::makePlayer(*whiteKind, std::cin, std::cout, seed);
    const auto blackPlayer = ui::makePlayer(*blackKind, std::cin, std::cout,
                                            seed ? std::optional<std::uint32_t>(*seed + 1) : std::nullopt);

    ui::GameOptions options;
    if (*whiteKind == ui::PlayerKind::Computer && *blackKind == ui::PlayerKind::Computer)
        options.maxPlies = kComputerOnlyPlyLimit;

    ui::runGame(*whitePlayer, *blackPlayer, std::cout, chess::Position::startingPosition(), options);
    return 0;
}
