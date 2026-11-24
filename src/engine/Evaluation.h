#pragma once
#include <cstdlib>
#include "../chess/Position.h"

namespace engine {

// All scores are in centipawns.
//
// SIGN CONVENTION: every score is from the point of view of the SIDE TO MOVE of the position
// being scored: positive = good for the player whose turn it is, negative = bad for them.
// This is the convention negamax needs: score(parent) = max over moves of -score(child).
constexpr int kPawnValue = 100;
constexpr int kKnightValue = 320;
constexpr int kBishopValue = 330;
constexpr int kRookValue = 500;
constexpr int kQueenValue = 900;
constexpr int kKingValue = 0;

// Score of a checkmated side to move at the root. At search ply p the score is -(kMateScore - p),
// so quicker mates score higher for the winner and unavoidable mates are delayed by the loser.
constexpr int kMateScore = 100000;
constexpr int kMateThreshold = kMateScore - 1000;  // |score| above this means "forced mate"

constexpr bool isMateScore(int score) { return score >= kMateThreshold || score <= -kMateThreshold; }
// Number of plies from the root to the mate, for a mate score.
constexpr int matePlies(int score) { return kMateScore - (score < 0 ? -score : score); }

int pieceValue(chess::PieceType type);

// Material-only static evaluation from the side to move's perspective. No positional terms.
int evaluate(const chess::Position& position);

}  // namespace engine
