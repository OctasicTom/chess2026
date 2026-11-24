#include "Search.h"
#include <algorithm>
#include <chrono>
#include <limits>
#include "../chess/Attacks.h"
#include "Evaluation.h"

namespace engine {

namespace {

int terminalScore(const chess::Position& position, int ply) {
    return chess::isKingInCheck(position, position.sideToMove) ? -(kMateScore - ply) : 0;
}

}  // namespace

int negamax(const chess::Position& position, int depth, int ply, std::uint64_t& nodes) {
    ++nodes;
    const chess::MoveList moves = chess::generateLegalMoves(position);
    if (moves.empty()) return terminalScore(position, ply);
    if (depth <= 0) return evaluate(position);

    int best = std::numeric_limits<int>::min();
    for (const chess::Move& move : moves) {
        chess::Position child = position;
        child.applyMove(move);
        best = std::max(best, -negamax(child, depth - 1, ply + 1, nodes));
    }
    return best;
}

SearchResult findBestMove(const chess::Position& position, int depth) {
    const auto start = std::chrono::steady_clock::now();
    depth = std::max(depth, 1);

    SearchResult result;
    result.stats.depth = depth;
    result.stats.nodes = 1;  // the root

    const chess::MoveList moves = chess::generateLegalMoves(position);
    if (moves.empty()) {
        result.score = terminalScore(position, 0);
    } else {
        result.score = std::numeric_limits<int>::min();
        for (const chess::Move& move : moves) {
            chess::Position child = position;
            child.applyMove(move);
            const int score = -negamax(child, depth - 1, 1, result.stats.nodes);
            if (score > result.score) {
                result.score = score;
                result.bestMove = move;
            }
        }
    }
    result.stats.seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    return result;
}

}  // namespace engine
