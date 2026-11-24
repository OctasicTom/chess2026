#include <chrono>
#include <iomanip>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <vector>
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

// Usage: Chess.exe --compare <depth> [fen]   (plain negamax vs alpha-beta, same positional evaluation)
int runCompare(int argc, char** argv) {
    const int depth = std::atoi(argv[2]);
    const auto position = chess::parseFen(argc >= 4 ? argv[3] : chess::kStartingFen);
    if (depth < 1 || !position) {
        std::cerr << "Usage: Chess --compare <depth >= 1> [fen]\n";
        return 2;
    }
    struct Row { const char* name; engine::SearchConfig config; };
    engine::SearchConfig plain = engine::advancedConfig(depth);
    plain.algorithm = engine::SearchAlgorithm::PlainNegamax;
    plain.moveOrdering = false;
    engine::SearchConfig unordered = engine::advancedConfig(depth);
    unordered.moveOrdering = false;
    std::vector<Row> rows;
    if (depth <= 4) rows.push_back({"plain negamax", plain});  // too slow beyond depth 4
    rows.push_back({"alpha-beta, no ordering", unordered});
    rows.push_back({"alpha-beta + ordering", engine::advancedConfig(depth)});

    std::cout << "Depth " << depth << " (positional evaluation)\n"
              << std::left << std::setw(26) << "Search" << std::setw(8) << "Move" << std::setw(8) << "Score"
              << std::setw(14) << "Nodes" << std::setw(14) << "Leaves" << std::setw(12) << "Cutoffs" << "Time (s)\n";
    for (const Row& row : rows) {
        const engine::SearchResult r = engine::search(*position, row.config);
        std::cout << std::left << std::setw(26) << row.name << std::setw(8) << chess::moveToString(r.bestMove)
                  << std::setw(8) << r.score << std::setw(14) << r.stats.nodes << std::setw(14) << r.stats.leafNodes
                  << std::setw(12) << r.stats.cutoffs << std::fixed << std::setprecision(3) << r.stats.seconds << "\n";
        std::cout.unsetf(std::ios::floatfield);
    }
    return 0;
}

// Asks for the player type of one side, plus a search depth if needed. False at end of input.
bool configureSide(const char* name, ui::PlayerKind& kind, int& depth) {
    const auto k = ui::promptPlayerKind(std::cin, std::cout, name);
    if (!k) return false;
    kind = *k;
    depth = ui::kDefaultSearchDepth;
    if (kind == ui::PlayerKind::SearchComputer || kind == ui::PlayerKind::BasicSearchComputer) {
        const auto d = ui::promptSearchDepth(std::cin, std::cout);
        if (!d) return false;
        depth = *d;
    }
    return true;
}

}  // namespace

// Usage: Chess.exe [--seed <n>]      (--seed makes random computer players reproducible)
//        Chess.exe --perft <depth> [fen]
//        Chess.exe --search <depth> [fen]   (basic engine)
//        Chess.exe --compare <depth> [fen]
int main(int argc, char** argv) {
    if (argc >= 3 && std::string(argv[1]) == "--perft") return runPerft(argc, argv);
    if (argc >= 3 && std::string(argv[1]) == "--search") return runSearch(argc, argv);
    if (argc >= 3 && std::string(argv[1]) == "--compare") return runCompare(argc, argv);

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

