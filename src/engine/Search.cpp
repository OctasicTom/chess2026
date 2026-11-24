#include "Search.h"
#include <algorithm>
#include <chrono>
#include "../chess/Attacks.h"
#include "Evaluation.h"
#include "MoveOrdering.h"

namespace engine {

namespace {

constexpr int kInfinity = kMateScore + 1000;

struct Context {
    EvaluationFunction evaluate;
    bool ordering;
    SearchStats& stats;
};

int terminalScore(const chess::Position& position, int ply) {
    return chess::isKingInCheck(position, position.sideToMove) ? -(kMateScore - ply) : 0;
}

int plainNegamax(const chess::Position& position, int depth, int ply, Context& ctx) {
    ++ctx.stats.nodes;
    const chess::MoveList moves = chess::generateLegalMoves(position);
    if (moves.empty()) return terminalScore(position, ply);
    if (depth <= 0) {
        ++ctx.stats.leafNodes;
        return ctx.evaluate(position);
    }

    int best = -kInfinity;
    for (const chess::Move& move : moves) {
        chess::Position child = position;
        child.applyMove(move);
        best = std::max(best, -plainNegamax(child, depth - 1, ply + 1, ctx));
    }
    return best;
}

// Fail-soft alpha-beta in negamax form: score = -search(child, depth - 1, -beta, -alpha).
int alphaBeta(const chess::Position& position, int depth, int ply, int alpha, int beta, Context& ctx) {
    ++ctx.stats.nodes;
    chess::MoveList moves = chess::generateLegalMoves(position);
    if (moves.empty()) return terminalScore(position, ply);
    if (depth <= 0) {
        ++ctx.stats.leafNodes;
        return ctx.evaluate(position);
    }
    if (ctx.ordering) orderMoves(position, moves);

    int best = -kInfinity;
    for (const chess::Move& move : moves) {
        chess::Position child = position;
        child.applyMove(move);
        const int score = -alphaBeta(child, depth - 1, ply + 1, -beta, -alpha, ctx);
        best = std::max(best, score);
        alpha = std::max(alpha, score);
        if (alpha >= beta) {
            ++ctx.stats.cutoffs;
            break;
        }
    }
    return best;
}

}  // namespace

SearchConfig basicConfig(int depth) {
    SearchConfig c;
    c.depth = depth;
    c.algorithm = SearchAlgorithm::PlainNegamax;
    c.evaluator = evaluateMaterial;
    c.moveOrdering = false;
    return c;
}

SearchConfig advancedConfig(int depth) {
    SearchConfig c;
    c.depth = depth;
    c.algorithm = SearchAlgorithm::AlphaBeta;
    c.evaluator = evaluate;
    c.moveOrdering = true;
    return c;
}

const char* describe(SearchAlgorithm algorithm) {
    return algorithm == SearchAlgorithm::AlphaBeta ? "alpha-beta" : "plain negamax";
}

SearchResult search(const chess::Position& position, const SearchConfig& config) {
    const auto start = std::chrono::steady_clock::now();
    const int depth = std::max(config.depth, 1);

    SearchResult result;
    result.stats.depth = depth;
    result.stats.nodes = 1;  // the root
    Context ctx{config.evaluator ? config.evaluator : evaluate, config.moveOrdering, result.stats};
    const bool pruning = config.algorithm == SearchAlgorithm::AlphaBeta;

    chess::MoveList moves = chess::generateLegalMoves(position);
    if (moves.empty()) {
        result.score = terminalScore(position, 0);
    } else {
        if (pruning && ctx.ordering) orderMoves(position, moves);
        int alpha = -kInfinity;
        result.score = -kInfinity;
        for (const chess::Move& move : moves) {
            chess::Position child = position;
            child.applyMove(move);
            const int score = pruning ? -alphaBeta(child, depth - 1, 1, -kInfinity, -alpha, ctx)
                                      : -plainNegamax(child, depth - 1, 1, ctx);
            if (score > result.score) {
                result.score = score;
                result.bestMove = move;
            }
            alpha = std::max(alpha, score);
        }
    }
    result.stats.seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    return result;
}

int negamax(const chess::Position& position, int depth, int ply, std::uint64_t& nodes) {
    SearchStats stats;
    stats.nodes = nodes;
    Context ctx{evaluateMaterial, false, stats};
    const int score = plainNegamax(position, depth, ply, ctx);
    nodes = stats.nodes;
    return score;
}

SearchResult findBestMove(const chess::Position& position, int depth) {
    return search(position, basicConfig(depth));
}

}  // namespace engine
