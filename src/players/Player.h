#pragma once
#include <vector>
#include "../chess/Position.h"

namespace players {

// Something that can pick a move. Knows nothing about how the game is run or displayed.
// `legalMoves` is the complete list of legal moves for position.sideToMove (never empty when called).
// Returning a null Move (Move::isNull()) means the player is leaving the game (e.g. human typed "quit").
class Player {
public:
    virtual ~Player() = default;
    virtual chess::Move chooseMove(const chess::Position& position,
                                   const std::vector<chess::Move>& legalMoves) = 0;
};

}  // namespace players
