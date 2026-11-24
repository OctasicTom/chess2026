#include <iostream>
#include "../src/chess/Notation.h"
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
    // Square conversion
    CHECK(squareToString(0) == "a1");
    CHECK(squareToString(1) == "b1");
    CHECK(squareToString(7) == "h1");
    CHECK(squareToString(8) == "a2");
    CHECK(squareToString(63) == "h8");
    CHECK(squareToString(64) == "??");
    for (int i = 0; i < 64; ++i) {
        auto s = squareFromString(squareToString(static_cast<Square>(i)));
        CHECK(s && *s == i);
    }
    CHECK(!squareFromString("i1"));
    CHECK(!squareFromString("a9"));
    CHECK(!squareFromString("a0"));
    CHECK(!squareFromString("A1"));
    CHECK(!squareFromString("a"));
    CHECK(!squareFromString("a11"));
    CHECK(!squareFromString(""));

    // Move parsing
    auto m = parseMove("e2e4");
    CHECK(m.ok() && m.move.from == 12 && m.move.to == 28 && m.move.promotion == PieceType::None);
    m = parseMove("g1f3");
    CHECK(m.ok() && m.move.from == 6 && m.move.to == 21);
    m = parseMove("e7e8q");
    CHECK(m.ok() && m.move.from == 52 && m.move.to == 60 && m.move.promotion == PieceType::Queen);
    CHECK(m.move.has(MoveFlagPromotion));
    CHECK(parseMove("a7a8r").move.promotion == PieceType::Rook);
    CHECK(parseMove("a7a8b").move.promotion == PieceType::Bishop);
    CHECK(parseMove("a7a8n").move.promotion == PieceType::Knight);
    CHECK(parseMove("  E2E4 \n").ok());
    CHECK(parseMove("").error == MoveParseError::BadLength);
    CHECK(parseMove("e2e").error == MoveParseError::BadLength);
    CHECK(parseMove("e2e4qq").error == MoveParseError::BadLength);
    CHECK(parseMove("z2e4").error == MoveParseError::BadSourceSquare);
    CHECK(parseMove("e2e9").error == MoveParseError::BadDestinationSquare);
    CHECK(parseMove("e7e8k").error == MoveParseError::BadPromotion);
    CHECK(parseMove("e7e8p").error == MoveParseError::BadPromotion);
    CHECK(parseMove("e2e2").error == MoveParseError::SameSquare);

    std::cout << (failures ? "Tests failed\n" : "All tests passed\n");
    return failures;
}

