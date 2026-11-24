#include "Position.h"

namespace chess {

Position Position::startingPosition() {
    Position p;
    p.board.setStartingPosition();
    p.sideToMove = Color::White;
    p.castlingRights = CastleWhiteKing | CastleWhiteQueen | CastleBlackKing | CastleBlackQueen;
    return p;
}

}  // namespace chess
