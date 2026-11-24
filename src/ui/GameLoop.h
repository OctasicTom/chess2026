#pragma once
#include <iosfwd>
#include "../chess/Position.h"

namespace ui {

// Human-vs-human console game starting from `start`. Runs until checkmate, stalemate,
// "quit", or end of input. Uses only the chess rules API; contains no rules itself.
void runHumanVsHuman(std::istream& in, std::ostream& out, chess::Position start);

}  // namespace ui
