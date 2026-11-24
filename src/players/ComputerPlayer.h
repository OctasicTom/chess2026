#pragma once
#include <iosfwd>
#include "../engine/Search.h"
#include "Player.h"

namespace players {

// Chooses moves with the fixed-depth search in src/engine and prints diagnostics.
class ComputerPlayer : public Player {
public:
    // Basic Milestone 7 engine: plain negamax, material-only evaluation.
    explicit ComputerPlayer(int depth = 3, std::ostream* diagnostics = nullptr);
    // Any search configuration, e.g. engine::advancedConfig(4).
    explicit ComputerPlayer(const engine::SearchConfig& config, std::ostream* diagnostics = nullptr);

    chess::Move chooseMove(const chess::Position& position,
                           const std::vector<chess::Move>& legalMoves) override;

    int depth() const { return config_.depth; }
    const engine::SearchConfig& config() const { return config_; }

private:
    engine::SearchConfig config_;
    std::ostream* diagnostics_;
};

}  // namespace players
