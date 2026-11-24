#pragma once
#include "../chess/Position.h"

namespace players {

// Abstract source of moves (human, engine, ...). No implementations yet.
class Player {
public:
    virtual ~Player() = default;
    virtual chess::Move chooseMove(const chess::Position& position) = 0;
};

}  // namespace players
