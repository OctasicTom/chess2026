#pragma once
#include <cstdlib>
#include "../chess/Position.h"

namespace engine {

// All scores are in centipawns.
//
// SIGN CONVENTION: evaluate() and evaluateMaterial() score from the point of view of the SIDE TO MOVE:
// positive = good for the player whose turn it is. This is what negamax needs:
// score(parent) = max over moves of -score(child).
// The per-colour term functions below (e.g. pawnStructureScore) are from the given colour's own
// point of view: higher is better for that colour.
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

// ---- Tunable evaluation weights -------------------------------------------------------------------
// Positional terms are deliberately small next to material (a pawn is 100, a knight 320).
constexpr int kDoubledPawnPenalty = 15;   // per extra pawn on a file
constexpr int kIsolatedPawnPenalty = 12;  // per pawn with no friendly pawn on an adjacent file
// Passed-pawn bonus by how far the pawn has advanced (index = ranks advanced from its own side, 0..7).
inline constexpr int kPassedPawnBonus[8] = {0, 20, 25, 35, 55, 80, 100, 0};
constexpr int kBishopPairBonus = 40;
// Centipawns per square a piece can move to (squares holding friendly pieces do not count).
constexpr int kKnightMobility = 4;
constexpr int kBishopMobility = 4;
constexpr int kRookMobility = 2;
constexpr int kQueenMobility = 1;
// King phase: total non-pawn material of BOTH sides. At or above kMiddlegameMaterial the king uses the
// middlegame table; at or below kEndgameMaterial the endgame table; in between it is interpolated.
constexpr int kMiddlegameMaterial = 4500;
constexpr int kEndgameMaterial = 1500;
constexpr int kPhaseScale = 256;  // endgameWeight() range: 0 (middlegame) .. kPhaseScale (endgame)
// Endgame "mop-up": when one side is clearly ahead, push the losing king to the edge and bring the
// winning king close, so material advantages can be converted.
constexpr int kMopUpMinAdvantage = 300;
constexpr int kMopUpEdgeWeight = 10;   // per step the losing king is from the centre (0..6)
constexpr int kMopUpCloseWeight = 4;   // per step of closeness between the kings (0..12)

int pieceValue(chess::PieceType type);

// ---- Evaluation terms (each from the given colour's perspective) ----------------------------------
int materialScore(const chess::Position& position, chess::Color color);
// Piece-square bonuses for every piece except the king. Tables are written from White's side and
// mirrored vertically for Black.
int pieceSquareScore(const chess::Position& position, chess::Color color);
// King placement: middlegame (safety) and endgame (activity) tables blended by endgameWeight().
int kingScore(const chess::Position& position, chess::Color color);
int mobilityScore(const chess::Position& position, chess::Color color);
int pawnStructureScore(const chess::Position& position, chess::Color color);  // doubled, isolated, passed
int bishopPairScore(const chess::Position& position, chess::Color color);

// 0 = full middlegame, kPhaseScale = full endgame (from total non-pawn material).
int endgameWeight(const chess::Position& position);
// From White's perspective (positive = White is converting an advantage).
int mopUpScore(const chess::Position& position);

// ---- Evaluators ------------------------------------------------------------------------------------
// Material only, side-to-move perspective (the Milestone 7 evaluation).
int evaluateMaterial(const chess::Position& position);
// Material + piece-square tables + king phase + mobility + pawn structure + bishop pair + mop-up,
// side-to-move perspective. Purely static: it never generates moves or searches.
int evaluate(const chess::Position& position);

}  // namespace engine
