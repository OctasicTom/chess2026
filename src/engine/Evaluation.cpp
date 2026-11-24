#include "Evaluation.h"

namespace engine {

int pieceValue(chess::PieceType type) {
    switch (type) {
        case chess::PieceType::Pawn: return kPawnValue;
        case chess::PieceType::Knight: return kKnightValue;
        case chess::PieceType::Bishop: return kBishopValue;
        case chess::PieceType::Rook: return kRookValue;
        case chess::PieceType::Queen: return kQueenValue;
        case chess::PieceType::King: return kKingValue;
        case chess::PieceType::None: break;
    }
    return 0;
}

int evaluate(const chess::Position& position) {
    int score = 0;
    for (int sq = 0; sq < chess::kNumSquares; ++sq) {
        const chess::Piece p = position.board.at(static_cast<chess::Square>(sq));
        if (p.empty()) continue;
        score += p.color == position.sideToMove ? pieceValue(p.type) : -pieceValue(p.type);
    }
    return score;
}

}  // namespace engine
