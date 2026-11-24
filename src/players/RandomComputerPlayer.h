#pragma once
#include <cstdint>
#include <random>
#include "Player.h"

namespace players {

// Picks uniformly at random from the legal moves. No evaluation, no search, no I/O.
class RandomComputerPlayer : public Player {
public:
    RandomComputerPlayer();                                  // seeded from std::random_device
    explicit RandomComputerPlayer(std::uint32_t seed);       // deterministic, for reproducible games

    chess::Move chooseMove(const chess::Position& position,
                           const std::vector<chess::Move>& legalMoves) override;

private:
    std::mt19937 rng_;
};

}  // namespace players
