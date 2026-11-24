#include "ComputerPlayer.h"
#include <algorithm>
#include <iomanip>
#include <ostream>
#include "../chess/Notation.h"
#include "../engine/Evaluation.h"

namespace players {

ComputerPlayer::ComputerPlayer(int depth, std::ostream* diagnostics)
    : ComputerPlayer(engine::basicConfig(std::max(depth, 1)), diagnostics) {}

ComputerPlayer::ComputerPlayer(const engine::SearchConfig& config, std::ostream* diagnostics)
    : config_(config), diagnostics_(diagnostics) {
    config_.depth = std::max(config_.depth, 1);
}

chess::Move ComputerPlayer::chooseMove(const chess::Position& position,
                                       const std::vector<chess::Move>& legalMoves) {
    if (legalMoves.empty()) return chess::Move{};
    const engine::SearchResult result = engine::search(position, config_);

    if (diagnostics_) {
        std::ostream& out = *diagnostics_;
        // Report from White's point of view, which is easier to follow than side-to-move scores.
        const bool whiteToMove = position.sideToMove == chess::Color::White;
        const int whiteScore = whiteToMove ? result.score : -result.score;
        out << "Computer plays: " << chess::moveToString(result.bestMove) << '\n';
        if (engine::isMateScore(result.score)) {
            const int moves = (engine::matePlies(result.score) + 1) / 2;
            const bool moverWins = result.score > 0;
            out << "Evaluation: mate in " << moves << " for "
                << ((moverWins == whiteToMove) ? "White" : "Black") << '\n';
        } else {
            out << "Evaluation: " << (whiteScore >= 0 ? "+" : "") << whiteScore << " (White's view)\n";
        }
        out << "Depth: " << result.stats.depth << " (" << engine::describe(config_.algorithm) << ")\n"
            << "Nodes searched: " << result.stats.nodes << '\n';
        if (config_.algorithm == engine::SearchAlgorithm::AlphaBeta)
            out << "Cutoffs: " << result.stats.cutoffs << '\n';
        out << "Time: " << std::fixed << std::setprecision(2) << result.stats.seconds << " seconds\n";
        out.unsetf(std::ios::floatfield);
    }
    return result.bestMove;
}

}  // namespace players
