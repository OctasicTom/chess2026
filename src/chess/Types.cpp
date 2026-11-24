#include "Types.h"

namespace chess {

char Piece::toChar() const {
    static const char letters[] = {'.', 'p', 'n', 'b', 'r', 'q', 'k'};
    if (empty()) return '.';
    char c = letters[static_cast<int>(type)];
    return color == Color::White ? static_cast<char>(c - 'a' + 'A') : c;
}

}  // namespace chess
