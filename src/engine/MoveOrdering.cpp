#include "MoveOrdering.h"
#include <algorithm>
#include <utility>
#include <vector>
#include "Evaluation.h"

namespace engine {

int moveOrderScore(const chess::Position& position, const chess::Move& move) {
    int score = 0;
    if (move.has(chess::MoveFlagCapture)) {
        const chess::PieceType victim = move.has(chess::MoveFlagEnPassant)
                                            ? chess::PieceType::Pawn
                                            : position.board.at(move.to).type;
        const chess::PieceType attacker = position.board.at(move.from).type;
        // PieceType values run Pawn(1) < Knight < Bishop < Rook < Queen < King(6).
        score += 10000 + static_cast<int>(victim) * 16 - static_cast<int>(attacker);
    }
    if (move.promotion != chess::PieceType::None) score += 9000 + pieceValue(move.promotion);
    return score;
}

void orderMoves(const chess::Position& position, chess::MoveList& moves) {
    std::vector<std::pair<int, chess::Move>> scored;
    scored.reserve(moves.size());
    for (const chess::Move& m : moves) scored.emplace_back(moveOrderScore(position, m), m);
    std::stable_sort(scored.begin(), scored.end(),
                     [](const std::pair<int, chess::Move>& a, const std::pair<int, chess::Move>& b) {
                         return a.first > b.first;
                     });
    for (std::size_t i = 0; i < moves.size(); ++i) moves[i] = scored[i].second;
}

}  // namespace engine
