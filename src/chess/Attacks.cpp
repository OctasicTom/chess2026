#include "Attacks.h"

namespace chess {

namespace {

bool onBoard(int file, int rank) { return file >= 0 && file < 8 && rank >= 0 && rank < 8; }

bool isPiece(const Position& pos, int file, int rank, PieceType type, Color color) {
    return onBoard(file, rank) && pos.board.at(makeSquare(file, rank)) == Piece{type, color};
}

// Walks outward along (df, dr) to the first occupied square; true if it holds an enemy slider
// of one of the two given types.
bool slidingAttack(const Position& pos, int file, int rank, int df, int dr,
                   PieceType a, PieceType b, Color attacker) {
    for (int f = file + df, r = rank + dr; onBoard(f, r); f += df, r += dr) {
        const Piece p = pos.board.at(makeSquare(f, r));
        if (p.empty()) continue;
        return p.color == attacker && (p.type == a || p.type == b);
    }
    return false;
}

}  // namespace

bool isSquareAttacked(const Position& position, int square, Color attackingColor) {
    if (square < 0 || square >= kNumSquares || attackingColor == Color::None) return false;
    const int file = fileOf(static_cast<Square>(square));
    const int rank = rankOf(static_cast<Square>(square));

    // A white pawn attacks upward, so it sits one rank below the target square (and vice versa).
    const int pawnRank = rank + (attackingColor == Color::White ? -1 : 1);
    if (isPiece(position, file - 1, pawnRank, PieceType::Pawn, attackingColor) ||
        isPiece(position, file + 1, pawnRank, PieceType::Pawn, attackingColor))
        return true;

    static const int knight[8][2] = {{1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};
    for (const auto& d : knight)
        if (isPiece(position, file + d[0], rank + d[1], PieceType::Knight, attackingColor)) return true;

    for (int df = -1; df <= 1; ++df)
        for (int dr = -1; dr <= 1; ++dr) {
            if (df == 0 && dr == 0) continue;
            if (isPiece(position, file + df, rank + dr, PieceType::King, attackingColor)) return true;
            const bool diagonal = df != 0 && dr != 0;
            if (slidingAttack(position, file, rank, df, dr,
                              diagonal ? PieceType::Bishop : PieceType::Rook, PieceType::Queen, attackingColor))
                return true;
        }
    return false;
}

bool isKingInCheck(const Position& position, Color kingColor) {
    if (kingColor == Color::None) return false;
    for (int sq = 0; sq < kNumSquares; ++sq)
        if (position.board.at(static_cast<Square>(sq)) == Piece{PieceType::King, kingColor})
            return isSquareAttacked(position, sq, opposite(kingColor));
    return false;
}

}  // namespace chess
