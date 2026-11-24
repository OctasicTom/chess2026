#include "ConsoleView.h"
#include <ostream>

namespace ui {

void printPosition(std::ostream& out, const chess::Position& position) {
    for (int rank = 7; rank >= 0; --rank) {
        out << (rank + 1) << "  ";
        for (int file = 0; file < 8; ++file)
            out << position.board.at(chess::makeSquare(file, rank)).toChar() << ' ';
        out << '\n';
    }
    out << "\n   a b c d e f g h\n";
    out << (position.sideToMove == chess::Color::White ? "White" : "Black") << " to move\n";
}

}  // namespace ui
