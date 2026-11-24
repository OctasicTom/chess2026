#include "Position.h"

namespace chess {

Position Position::startingPosition() {
    Position p;
    p.board.setStartingPosition();
    p.sideToMove = Color::White;
    p.castlingRights = CastleWhiteKing | CastleWhiteQueen | CastleBlackKing | CastleBlackQueen;
    return p;
}

namespace {

// Castling rights lost when a piece moves from or to the given square (covers king moves,
// rook moves, and rooks captured on their original squares).
std::uint8_t castlingMask(Square s) {
    switch (s) {
        case 0:  return CastleWhiteQueen;
        case 4:  return CastleWhiteKing | CastleWhiteQueen;
        case 7:  return CastleWhiteKing;
        case 56: return CastleBlackQueen;
        case 60: return CastleBlackKing | CastleBlackQueen;
        case 63: return CastleBlackKing;
        default: return 0;
    }
}

}  // namespace

void Position::applyMove(const Move& move) {
    if (move.isNull() || move.from >= kNumSquares || move.to >= kNumSquares) return;
    const Piece moving = board.at(move.from);
    if (moving.empty()) return;

    // Special moves are inferred from the board so that moves parsed from text (which carry no
    // flags) are applied correctly too.
    const int fromFile = fileOf(move.from), fromRank = rankOf(move.from);
    const int toFile = fileOf(move.to), toRank = rankOf(move.to);
    bool capture = !board.at(move.to).empty();
    Square newEnPassant = kNoSquare;
    Piece placed = moving;

    if (moving.type == PieceType::Pawn) {
        if (fromFile != toFile && !capture) {  // en passant: captured pawn is beside the origin
            board.set(makeSquare(toFile, fromRank), Piece{});
            capture = true;
        }
        if (toRank - fromRank == 2 || fromRank - toRank == 2)
            newEnPassant = makeSquare(fromFile, (fromRank + toRank) / 2);
        if ((toRank == 7 || toRank == 0) && move.promotion != PieceType::None)
            placed.type = move.promotion;
    } else if (moving.type == PieceType::King && (toFile - fromFile == 2 || fromFile - toFile == 2)) {
        const bool kingside = toFile > fromFile;
        const Square rookFrom = makeSquare(kingside ? 7 : 0, fromRank);
        const Square rookTo = makeSquare(kingside ? 5 : 3, fromRank);
        board.set(rookTo, board.at(rookFrom));
        board.set(rookFrom, Piece{});
    }

    board.set(move.to, placed);
    board.set(move.from, Piece{});

    castlingRights = static_cast<std::uint8_t>(castlingRights & ~(castlingMask(move.from) | castlingMask(move.to)));
    enPassantSquare = newEnPassant;
    halfmoveClock = (capture || moving.type == PieceType::Pawn) ? 0 : halfmoveClock + 1;
    if (sideToMove == Color::Black) ++fullmoveNumber;
    sideToMove = opposite(sideToMove);
}

}  // namespace chess
