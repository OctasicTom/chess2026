#pragma once
#include <iosfwd>
#include "Player.h"

namespace players {

// Reads coordinate-notation moves from a text stream until a legal one is entered.
// Returns a null Move on "quit" or end of input.
class HumanPlayer : public Player {
public:
    HumanPlayer(std::istream& in, std::ostream& out) : in_(in), out_(out) {}

    chess::Move chooseMove(const chess::Position& position,
                           const std::vector<chess::Move>& legalMoves) override;

private:
    std::istream& in_;
    std::ostream& out_;
};

}  // namespace players
