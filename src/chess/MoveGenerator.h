#pragma once
#include <vector>
#include "Position.h"

namespace chess {

using MoveList = std::vector<Move>;

// Pseudo-legal moves for position.sideToMove. Does not consider check, pins, castling,
// en passant or promotion (a pawn reaching the last rank yields a plain, non-promoting move).
MoveList generatePseudoLegalMoves(const Position& position);

// Fully legal moves for the supported rules: pseudo-legal moves that do not leave the mover's
// king in check.
MoveList generateLegalMoves(const Position& position);

enum class GameStatus { Ongoing, Checkmate, Stalemate };

GameStatus getGameStatus(const Position& position);

const char* describe(GameStatus status);

}  // namespace chess

