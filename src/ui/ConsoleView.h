#pragma once
#include <iosfwd>
#include "../chess/Position.h"

namespace ui {

void printPosition(std::ostream& out, const chess::Position& position);

}  // namespace ui
