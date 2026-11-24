#pragma once
#include "Position.h"

namespace chess {

// True if `attackingColor` has a piece attacking `square` (regardless of what occupies it).
bool isSquareAttacked(const Position& position, int square, Color attackingColor);

// True if the king of `kingColor` is attacked by the opponent. False if that side has no king.
bool isKingInCheck(const Position& position, Color kingColor);

}  // namespace chess
