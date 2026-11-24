#pragma once
#include <cstdint>
#include <utility>
#include <vector>
#include "MoveGenerator.h"

namespace chess {

// Counts leaf nodes of the legal-move tree to the given depth. Move-generation validation only;
// it knows nothing about evaluation or search.
std::uint64_t perft(const Position& position, int depth);

// Per-root-move node counts (depth >= 1), for locating generator bugs.
std::vector<std::pair<Move, std::uint64_t>> perftDivide(const Position& position, int depth);

}  // namespace chess
