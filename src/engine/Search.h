#pragma once
#include <cstdint>
#include "../chess/MoveGenerator.h"

namespace engine {

// Statistics are kept apart from the algorithm so different searches can be compared later.
struct SearchStats {
    int depth = 0;
    std::uint64_t nodes = 0;  // every position visited, including the root and leaves
    double seconds = 0.0;
};

struct SearchResult {
    chess::Move bestMove;  // null if the root position has no legal moves
    int score = 0;         // side-to-move perspective (see Evaluation.h)
    SearchStats stats;
};

// Plain fixed-depth negamax. `ply` is the distance from the root (used for mate scores) and `nodes`
// is incremented once per call. Returns a score from the side to move's perspective.
// Terminal positions are scored before depth is considered: checkmate -(kMateScore - ply),
// stalemate 0. At depth 0 the position is scored by evaluate().
int negamax(const chess::Position& position, int depth, int ply, std::uint64_t& nodes);

// Searches every legal root move to `depth` plies (clamped to at least 1) and returns the best one.
// Ties keep the first move in generation order, so results are deterministic.
SearchResult findBestMove(const chess::Position& position, int depth);

}  // namespace engine
