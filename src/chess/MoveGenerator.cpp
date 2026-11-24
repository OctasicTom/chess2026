#include "MoveGenerator.h"
#include "Attacks.h"

namespace chess {

namespace {

struct Delta { int df, dr; };

const Delta kKnightDeltas[] = {{1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};
const Delta kDiagonalDeltas[] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
const Delta kOrthogonalDeltas[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
const PieceType kPromotionTypes[] = {PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight};

bool onBoard(int file, int rank) { return file >= 0 && file < 8 && rank >= 0 && rank < 8; }

class Generator {
public:
    Generator(const Position& p, MoveList& out) : pos_(p), out_(out), us_(p.sideToMove) {}

    void run() {
        for (int sq = 0; sq < kNumSquares; ++sq) {
            const Piece piece = pos_.board.at(static_cast<Square>(sq));
            if (piece.empty() || piece.color != us_) continue;
            const Square from = static_cast<Square>(sq);
            switch (piece.type) {
                case PieceType::Pawn: pawn(from); break;
                case PieceType::Knight: step(from, kKnightDeltas); break;
                case PieceType::Bishop: slide(from, kDiagonalDeltas); break;
                case PieceType::Rook: slide(from, kOrthogonalDeltas); break;
                case PieceType::Queen: slide(from, kDiagonalDeltas); slide(from, kOrthogonalDeltas); break;
                case PieceType::King:
                    step(from, kDiagonalDeltas);
                    step(from, kOrthogonalDeltas);
                    castling();
                    break;
                case PieceType::None: break;
            }
        }
    }

private:
    void add(Square from, Square to, int flags, PieceType promotion = PieceType::None) {
        out_.push_back(Move{from, to, promotion, static_cast<std::uint8_t>(flags)});
    }

    // Adds a move if the target is empty or an enemy piece. Returns true if the target was empty.
    bool addIfAllowed(Square from, int file, int rank) {
        if (!onBoard(file, rank)) return false;
        const Square to = makeSquare(file, rank);
        const Piece target = pos_.board.at(to);
        if (target.empty()) {
            add(from, to, MoveFlagNone);
            return true;
        }
        if (target.color != us_) add(from, to, MoveFlagCapture);
        return false;
    }

    template <std::size_t N>
    void step(Square from, const Delta (&deltas)[N]) {
        for (const Delta& d : deltas) addIfAllowed(from, fileOf(from) + d.df, rankOf(from) + d.dr);
    }

    template <std::size_t N>
    void slide(Square from, const Delta (&deltas)[N]) {
        for (const Delta& d : deltas) {
            int f = fileOf(from) + d.df, r = rankOf(from) + d.dr;
            while (addIfAllowed(from, f, r)) { f += d.df; r += d.dr; }
        }
    }

    // Adds a pawn move, expanding into the four promotion choices on the last rank.
    void pawnMove(Square from, Square to, int flags) {
        const int promoRank = us_ == Color::White ? 7 : 0;
        if (rankOf(to) == promoRank) {
            for (PieceType t : kPromotionTypes) add(from, to, flags | MoveFlagPromotion, t);
        } else {
            add(from, to, flags);
        }
    }

    void pawn(Square from) {
        const int dir = us_ == Color::White ? 1 : -1;
        const int startRank = us_ == Color::White ? 1 : 6;
        const int f = fileOf(from), r = rankOf(from);

        if (onBoard(f, r + dir) && pos_.board.at(makeSquare(f, r + dir)).empty()) {
            pawnMove(from, makeSquare(f, r + dir), MoveFlagNone);
            if (r == startRank && pos_.board.at(makeSquare(f, r + 2 * dir)).empty())
                add(from, makeSquare(f, r + 2 * dir), MoveFlagDoublePush);
        }
        for (int df : {-1, 1}) {
            if (!onBoard(f + df, r + dir)) continue;
            const Square to = makeSquare(f + df, r + dir);
            const Piece target = pos_.board.at(to);
            if (!target.empty()) {
                if (target.color != us_) pawnMove(from, to, MoveFlagCapture);
            } else if (to == pos_.enPassantSquare &&
                       pos_.board.at(makeSquare(f + df, r)) == Piece{PieceType::Pawn, opposite(us_)}) {
                add(from, to, MoveFlagCapture | MoveFlagEnPassant);
            }
        }
    }

    bool empty(int file, int rank) const { return pos_.board.at(makeSquare(file, rank)).empty(); }
    bool attacked(int file, int rank) const {
        return isSquareAttacked(pos_, makeSquare(file, rank), opposite(us_));
    }

    void castling() {
        const bool white = us_ == Color::White;
        const int rank = white ? 0 : 7;
        const Square kingSq = makeSquare(4, rank);
        if (pos_.board.at(kingSq) != Piece{PieceType::King, us_}) return;
        const Piece ourRook{PieceType::Rook, us_};
        const int kingRight = white ? CastleWhiteKing : CastleBlackKing;
        const int queenRight = white ? CastleWhiteQueen : CastleBlackQueen;
        if (!(pos_.castlingRights & (kingRight | queenRight))) return;
        if (attacked(4, rank)) return;

        if ((pos_.castlingRights & kingRight) && pos_.board.at(makeSquare(7, rank)) == ourRook &&
            empty(5, rank) && empty(6, rank) && !attacked(5, rank) && !attacked(6, rank))
            add(kingSq, makeSquare(6, rank), MoveFlagCastleKing);

        if ((pos_.castlingRights & queenRight) && pos_.board.at(makeSquare(0, rank)) == ourRook &&
            empty(1, rank) && empty(2, rank) && empty(3, rank) && !attacked(3, rank) && !attacked(2, rank))
            add(kingSq, makeSquare(2, rank), MoveFlagCastleQueen);
    }

    const Position& pos_;
    MoveList& out_;
    Color us_;
};

}  // namespace

MoveList generatePseudoLegalMoves(const Position& position) {
    MoveList moves;
    moves.reserve(48);
    if (position.sideToMove == Color::None) return moves;
    Generator(position, moves).run();
    return moves;
}

MoveList generateLegalMoves(const Position& position) {
    const Color mover = position.sideToMove;
    MoveList legal;
    for (const Move& move : generatePseudoLegalMoves(position)) {
        Position next = position;
        next.applyMove(move);
        if (!isKingInCheck(next, mover)) legal.push_back(move);
    }
    return legal;
}

GameStatus getGameStatus(const Position& position) {
    if (!generateLegalMoves(position).empty()) return GameStatus::Ongoing;
    return isKingInCheck(position, position.sideToMove) ? GameStatus::Checkmate : GameStatus::Stalemate;
}

const char* describe(GameStatus status) {
    switch (status) {
        case GameStatus::Ongoing: return "ongoing";
        case GameStatus::Checkmate: return "CHECKMATE";
        case GameStatus::Stalemate: return "STALEMATE";
    }
    return "unknown";
}

}  // namespace chess
