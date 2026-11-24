#include "RandomComputerPlayer.h"

namespace players {

RandomComputerPlayer::RandomComputerPlayer() : rng_(std::random_device{}()) {}

RandomComputerPlayer::RandomComputerPlayer(std::uint32_t seed) : rng_(seed) {}

chess::Move RandomComputerPlayer::chooseMove(const chess::Position&,
                                             const std::vector<chess::Move>& legalMoves) {
    if (legalMoves.empty()) return chess::Move{};
    std::uniform_int_distribution<std::size_t> pick(0, legalMoves.size() - 1);
    return legalMoves[pick(rng_)];
}

}  // namespace players
