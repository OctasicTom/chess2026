#include "Perft.h"

namespace chess {

std::uint64_t perft(const Position& position, int depth) {
    if (depth <= 0) return 1;
    const MoveList moves = generateLegalMoves(position);
    if (depth == 1) return moves.size();

    std::uint64_t nodes = 0;
    for (const Move& move : moves) {
        Position next = position;
        next.applyMove(move);
        nodes += perft(next, depth - 1);
    }
    return nodes;
}

std::vector<std::pair<Move, std::uint64_t>> perftDivide(const Position& position, int depth) {
    std::vector<std::pair<Move, std::uint64_t>> result;
    for (const Move& move : generateLegalMoves(position)) {
        Position next = position;
        next.applyMove(move);
        result.emplace_back(move, perft(next, depth - 1));
    }
    return result;
}

}  // namespace chess
