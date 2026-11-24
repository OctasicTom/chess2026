#include "Evaluation.h"
#include <algorithm>

namespace engine {

using namespace chess;

namespace {

// Piece-square tables, written as the board looks to White: first row = rank 8, last row = rank 1.
const int kPawnTable[64] = {
      0,  0,  0,  0,  0,  0,  0,  0,
     50, 50, 50, 50, 50, 50, 50, 50,
     10, 10, 20, 30, 30, 20, 10, 10,
      5,  5, 10, 25, 25, 10,  5,  5,
      0,  0,  0, 20, 20,  0,  0,  0,
      5, -5,-10,  0,  0,-10, -5,  5,
      5, 10, 10,-20,-20, 10, 10,  5,
      0,  0,  0,  0,  0,  0,  0,  0};
const int kKnightTable[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50};
const int kBishopTable[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20};
const int kRookTable[64] = {
      0,  0,  0,  0,  0,  0,  0,  0,
      5, 10, 10, 10, 10, 10, 10,  5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
      0,  0,  0,  5,  5,  0,  0,  0};
const int kQueenTable[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20};
const int kKingMiddlegameTable[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20};
const int kKingEndgameTable[64] = {
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50};

// Index into a table for a piece of `color` on `square`. White reads the table as written; Black
// reads it flipped vertically, so both sides see their own back rank at the bottom of the table.
int tableIndex(Square square, Color color) {
    const int file = fileOf(square), rank = rankOf(square);
    return color == Color::White ? (7 - rank) * 8 + file : rank * 8 + file;
}

const int* tableFor(PieceType type) {
    switch (type) {
        case PieceType::Pawn: return kPawnTable;
        case PieceType::Knight: return kKnightTable;
        case PieceType::Bishop: return kBishopTable;
        case PieceType::Rook: return kRookTable;
        case PieceType::Queen: return kQueenTable;
        default: return nullptr;
    }
}

bool onBoard(int file, int rank) { return file >= 0 && file < 8 && rank >= 0 && rank < 8; }

Square findKing(const Position& position, Color color) {
    for (int sq = 0; sq < kNumSquares; ++sq)
        if (position.board.at(static_cast<Square>(sq)) == Piece{PieceType::King, color})
            return static_cast<Square>(sq);
    return kNoSquare;
}

int nonPawnMaterial(const Position& position, Color color) {
    int total = 0;
    for (int sq = 0; sq < kNumSquares; ++sq) {
        const Piece p = position.board.at(static_cast<Square>(sq));
        if (p.color == color && p.type != PieceType::Pawn && p.type != PieceType::King)
            total += pieceValue(p.type);
    }
    return total;
}

int mobilityWeight(PieceType type) {
    switch (type) {
        case PieceType::Knight: return kKnightMobility;
        case PieceType::Bishop: return kBishopMobility;
        case PieceType::Rook: return kRookMobility;
        case PieceType::Queen: return kQueenMobility;
        default: return 0;
    }
}

const int kKnightSteps[8][2] = {{1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};
const int kDiagonals[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
const int kOrthogonals[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

// Squares the piece can reach (empty, or holding an enemy piece). Pseudo-mobility: ignores pins and
// checks, and uses no move generation, so evaluation never recurses into the rules code.
int reachableSquares(const Position& position, Square from, PieceType type, Color color) {
    const int file = fileOf(from), rank = rankOf(from);
    int count = 0;
    if (type == PieceType::Knight) {
        for (const auto& d : kKnightSteps) {
            const int f = file + d[0], r = rank + d[1];
            if (onBoard(f, r) && position.board.at(makeSquare(f, r)).color != color) ++count;
        }
        return count;
    }
    const bool diagonal = type == PieceType::Bishop || type == PieceType::Queen;
    const bool straight = type == PieceType::Rook || type == PieceType::Queen;
    for (int pass = 0; pass < 2; ++pass) {
        if ((pass == 0 && !diagonal) || (pass == 1 && !straight)) continue;
        for (int i = 0; i < 4; ++i) {
            const int df = pass == 0 ? kDiagonals[i][0] : kOrthogonals[i][0];
            const int dr = pass == 0 ? kDiagonals[i][1] : kOrthogonals[i][1];
            for (int f = file + df, r = rank + dr; onBoard(f, r); f += df, r += dr) {
                const Piece target = position.board.at(makeSquare(f, r));
                if (target.color == color) break;
                ++count;
                if (!target.empty()) break;
            }
        }
    }
    return count;
}

int centreDistance(Square s) {  // 0 in the centre .. 6 in a corner
    const int f = fileOf(s), r = rankOf(s);
    return (3 - std::min(f, 7 - f)) + (3 - std::min(r, 7 - r));
}

}  // namespace

int pieceValue(PieceType type) {
    switch (type) {
        case PieceType::Pawn: return kPawnValue;
        case PieceType::Knight: return kKnightValue;
        case PieceType::Bishop: return kBishopValue;
        case PieceType::Rook: return kRookValue;
        case PieceType::Queen: return kQueenValue;
        case PieceType::King: return kKingValue;
        case PieceType::None: break;
    }
    return 0;
}

int materialScore(const Position& position, Color color) {
    int total = 0;
    for (int sq = 0; sq < kNumSquares; ++sq) {
        const Piece p = position.board.at(static_cast<Square>(sq));
        if (p.color == color) total += pieceValue(p.type);
    }
    return total;
}

int pieceSquareScore(const Position& position, Color color) {
    int total = 0;
    for (int sq = 0; sq < kNumSquares; ++sq) {
        const Piece p = position.board.at(static_cast<Square>(sq));
        if (p.empty() || p.color != color) continue;
        if (const int* table = tableFor(p.type)) total += table[tableIndex(static_cast<Square>(sq), color)];
    }
    return total;
}

int endgameWeight(const Position& position) {
    const int material = nonPawnMaterial(position, Color::White) + nonPawnMaterial(position, Color::Black);
    if (material >= kMiddlegameMaterial) return 0;
    if (material <= kEndgameMaterial) return kPhaseScale;
    return kPhaseScale * (kMiddlegameMaterial - material) / (kMiddlegameMaterial - kEndgameMaterial);
}

int kingScore(const Position& position, Color color) {
    const Square king = findKing(position, color);
    if (king == kNoSquare) return 0;
    const int index = tableIndex(king, color);
    const int w = endgameWeight(position);
    return (kKingMiddlegameTable[index] * (kPhaseScale - w) + kKingEndgameTable[index] * w) / kPhaseScale;
}

int mobilityScore(const Position& position, Color color) {
    int total = 0;
    for (int sq = 0; sq < kNumSquares; ++sq) {
        const Piece p = position.board.at(static_cast<Square>(sq));
        if (p.empty() || p.color != color) continue;
        const int weight = mobilityWeight(p.type);
        if (weight) total += weight * reachableSquares(position, static_cast<Square>(sq), p.type, color);
    }
    return total;
}

int pawnStructureScore(const Position& position, Color color) {
    const Piece ours{PieceType::Pawn, color};
    const Piece theirs{PieceType::Pawn, opposite(color)};
    int ourPawnsOnFile[8] = {};
    for (int sq = 0; sq < kNumSquares; ++sq)
        if (position.board.at(static_cast<Square>(sq)) == ours) ++ourPawnsOnFile[fileOf(static_cast<Square>(sq))];

    int score = 0;
    for (int file = 0; file < 8; ++file)
        if (ourPawnsOnFile[file] > 1) score -= kDoubledPawnPenalty * (ourPawnsOnFile[file] - 1);

    const int forward = color == Color::White ? 1 : -1;
    for (int sq = 0; sq < kNumSquares; ++sq) {
        const Square s = static_cast<Square>(sq);
        if (position.board.at(s) != ours) continue;
        const int file = fileOf(s), rank = rankOf(s);

        const bool hasNeighbour = (file > 0 && ourPawnsOnFile[file - 1] > 0) ||
                                  (file < 7 && ourPawnsOnFile[file + 1] > 0);
        if (!hasNeighbour) score -= kIsolatedPawnPenalty;

        bool passed = true;
        for (int f = std::max(file - 1, 0); f <= std::min(file + 1, 7) && passed; ++f)
            for (int r = rank + forward; r >= 0 && r < 8; r += forward)
                if (position.board.at(makeSquare(f, r)) == theirs) { passed = false; break; }
        if (passed) score += kPassedPawnBonus[color == Color::White ? rank : 7 - rank];
    }
    return score;
}

int bishopPairScore(const Position& position, Color color) {
    int bishops = 0;
    for (int sq = 0; sq < kNumSquares; ++sq)
        if (position.board.at(static_cast<Square>(sq)) == Piece{PieceType::Bishop, color}) ++bishops;
    return bishops >= 2 ? kBishopPairBonus : 0;
}

int mopUpScore(const Position& position) {
    const int w = endgameWeight(position);
    if (w == 0) return 0;
    const int diff = materialScore(position, Color::White) - materialScore(position, Color::Black);
    if (diff > -kMopUpMinAdvantage && diff < kMopUpMinAdvantage) return 0;
    const Color winner = diff > 0 ? Color::White : Color::Black;
    const Square winnerKing = findKing(position, winner);
    const Square loserKing = findKing(position, opposite(winner));
    if (winnerKing == kNoSquare || loserKing == kNoSquare) return 0;

    const int distance = std::abs(fileOf(winnerKing) - fileOf(loserKing)) +
                         std::abs(rankOf(winnerKing) - rankOf(loserKing));  // 1..14
    const int bonus = (centreDistance(loserKing) * kMopUpEdgeWeight + (14 - distance) * kMopUpCloseWeight) *
                      w / kPhaseScale;
    return winner == Color::White ? bonus : -bonus;
}

int evaluateMaterial(const Position& position) {
    const Color us = position.sideToMove;
    return materialScore(position, us) - materialScore(position, opposite(us));
}

int evaluate(const Position& position) {
    int white = 0, black = 0;
    white += materialScore(position, Color::White) + pieceSquareScore(position, Color::White) +
             kingScore(position, Color::White) + mobilityScore(position, Color::White) +
             pawnStructureScore(position, Color::White) + bishopPairScore(position, Color::White);
    black += materialScore(position, Color::Black) + pieceSquareScore(position, Color::Black) +
             kingScore(position, Color::Black) + mobilityScore(position, Color::Black) +
             pawnStructureScore(position, Color::Black) + bishopPairScore(position, Color::Black);
    const int whiteView = white - black + mopUpScore(position);
    return position.sideToMove == Color::White ? whiteView : -whiteView;
}

}  // namespace engine
