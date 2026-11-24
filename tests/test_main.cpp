#include <iostream>
#include "../src/chess/Position.h"

static int failures = 0;
#define CHECK(c) do { if (!(c)) { std::cerr << "FAIL: " #c "\n"; ++failures; } } while (0)

int main() {
    using namespace chess;
    const Position p = Position::startingPosition();
    CHECK(p.sideToMove == Color::White);
    CHECK(p.board.at(makeSquare(4, 0)) == (Piece{PieceType::King, Color::White}));
    CHECK(p.board.at(makeSquare(3, 7)) == (Piece{PieceType::Queen, Color::Black}));
    CHECK(p.board.at(makeSquare(4, 4)).empty());
    std::cout << (failures ? "Tests failed\n" : "All tests passed\n");
    return failures;
}
