#include "Board.h"

namespace chess {

void Board::setStartingPosition() {
    clear();
    static const PieceType backRank[8] = {
        PieceType::Rook, PieceType::Knight, PieceType::Bishop, PieceType::Queen,
        PieceType::King, PieceType::Bishop, PieceType::Knight, PieceType::Rook};
    for (int f = 0; f < 8; ++f) {
        set(makeSquare(f, 0), Piece{backRank[f], Color::White});
        set(makeSquare(f, 1), Piece{PieceType::Pawn, Color::White});
        set(makeSquare(f, 6), Piece{PieceType::Pawn, Color::Black});
        set(makeSquare(f, 7), Piece{backRank[f], Color::Black});
    }
}

}  // namespace chess
