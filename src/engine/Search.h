#pragma once
#include <cstdint>
#include "../chess/MoveGenerator.h"

namespace engine {

// Statistics are kept apart from the algorithm so different searches can be compared.
struct SearchStats {
    int depth = 0;
    std::uint64_t nodes = 0;      // every position visited, including the root and leaves
    double seconds = 0.0;
    std::uint64_t leafNodes = 0;  // positions scored by the evaluator at depth 0
    std::uint64_t cutoffs = 0;    // alpha-beta beta cutoffs (always 0 for plain negamax)
};

struct SearchResult {
    chess::Move bestMove;  // null if the root position has no legal moves
    int score = 0;         // side-to-move perspective (see Evaluation.h)
    SearchStats stats;
};

using EvaluationFunction = int (*)(const chess::Position&);

enum class SearchAlgorithm { PlainNegamax, AlphaBeta };

struct SearchConfig {
    int depth = 3;
    SearchAlgorithm algorithm = SearchAlgorithm::AlphaBeta;
    EvaluationFunction evaluator = nullptr;  // nullptr = evaluate() (full positional evaluation)
    bool moveOrdering = true;                // alpha-beta only; plain negamax never reorders
};

// The Milestone 7 baseline: plain negamax, material-only evaluation.
SearchConfig basicConfig(int depth);
// Alpha-beta with move ordering and the full positional evaluation.
SearchConfig advancedConfig(int depth);

const char* describe(SearchAlgorithm algorithm);

// Fixed-depth search. Terminal positions are scored before depth is considered: checkmate
// -(kMateScore - ply), stalemate 0. At depth 0 the position is scored by the evaluator.
// `depth` is clamped to at least 1. Alpha-beta returns the same score as plain negamax for the same
// evaluator and depth; when several moves tie, the chosen move may differ.
SearchResult search(const chess::Position& position, const SearchConfig& config);

// Plain negamax, material evaluation (Milestone 7 API, unchanged). `ply` is the distance from the root
// and `nodes` is incremented once per call.
int negamax(const chess::Position& position, int depth, int ply, std::uint64_t& nodes);

// search() with basicConfig(depth). Ties keep the first move in generation order.
SearchResult findBestMove(const chess::Position& position, int depth);

}  // namespace engine
