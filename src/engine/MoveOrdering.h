#pragma once
#include "../chess/MoveGenerator.h"

namespace engine {

// Orders moves so that likely-best ones come first (helps alpha-beta cut off early):
// captures by MVV-LVA (most valuable victim, least valuable attacker), then promotions, then the
// rest in their original order. The sort is stable. Checkmating moves are not singled out: finding
// them would need a full legal-move generation per candidate, which costs more than it saves.
void orderMoves(const chess::Position& position, chess::MoveList& moves);

// The heuristic score used for ordering (higher = searched earlier).
int moveOrderScore(const chess::Position& position, const chess::Move& move);

}  // namespace engine
