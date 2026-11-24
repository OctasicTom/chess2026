#pragma once
#include <array>
#include "Types.h"

namespace chess {

// Pure piece placement: a 64-element array, no game state.
class Board {
public:
    Board() { clear(); }

    void clear() { squares_.fill(Piece{}); }
    void setStartingPosition();

    Piece at(Square s) const { return squares_[s]; }
    void set(Square s, Piece p) { squares_[s] = p; }

private:
    std::array<Piece, kNumSquares> squares_;
};

}  // namespace chess
