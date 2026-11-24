#pragma once
#include <iosfwd>
#include "Player.h"

namespace players {

// Chooses moves with the fixed-depth negamax search in src/engine and prints diagnostics.
class ComputerPlayer : public Player {
public:
    explicit ComputerPlayer(int depth = 3, std::ostream* diagnostics = nullptr);

    chess::Move chooseMove(const chess::Position& position,
                           const std::vector<chess::Move>& legalMoves) override;

    int depth() const { return depth_; }

private:
    int depth_;
    std::ostream* diagnostics_;
};

}  // namespace players
