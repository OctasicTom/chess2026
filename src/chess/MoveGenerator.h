#pragma once
#include <vector>
#include "Position.h"

namespace chess {

using MoveList = std::vector<Move>;

// Pseudo-legal moves for position.sideToMove, including castling (fully checked), en passant and
// promotion (four moves per promoting pawn move). Moves that leave the king in check are included.
MoveList generatePseudoLegalMoves(const Position& position);

// Fully legal moves for the supported rules: pseudo-legal moves that do not leave the mover's
// king in check.
MoveList generateLegalMoves(const Position& position);

enum class GameStatus { Ongoing, Checkmate, Stalemate };

GameStatus getGameStatus(const Position& position);

const char* describe(GameStatus status);

}  // namespace chess


