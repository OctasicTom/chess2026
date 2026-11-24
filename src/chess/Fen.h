#pragma once
#include <optional>
#include <string>
#include <string_view>
#include "Position.h"

namespace chess {

constexpr const char* kStartingFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// Parses a FEN string. The halfmove/fullmove fields may be omitted (defaults 0 and 1).
// Returns nullopt for malformed input.
std::optional<Position> parseFen(std::string_view fen);

std::string toFen(const Position& position);

}  // namespace chess
