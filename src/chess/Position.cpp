#include "Position.h"

namespace chess {

Position Position::startingPosition() {
    Position p;
    p.board.setStartingPosition();
    p.sideToMove = Color::White;
    p.castlingRights = CastleWhiteKing | CastleWhiteQueen | CastleBlackKing | CastleBlackQueen;
    return p;
}

void Position::applyMove(const Move& move) {
    if (move.isNull() || move.from >= kNumSquares || move.to >= kNumSquares) return;
    const Piece moving = board.at(move.from);
    if (moving.empty()) return;
    const bool capture = !board.at(move.to).empty();

    board.set(move.to, moving);
    board.set(move.from, Piece{});

    enPassantSquare = kNoSquare;
    halfmoveClock = (capture || moving.type == PieceType::Pawn) ? 0 : halfmoveClock + 1;
    if (sideToMove == Color::Black) ++fullmoveNumber;
    sideToMove = opposite(sideToMove);
}

}  // namespace chess
