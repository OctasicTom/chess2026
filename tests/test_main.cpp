#include <iostream>
#include <algorithm>
#include <set>
#include <string>
#include "../src/chess/Attacks.h"
#include "../src/chess/MoveGenerator.h"
#include "../src/chess/Notation.h"
#include "../src/chess/Position.h"

static int failures = 0;
#define CHECK(c) do { if (!(c)) { std::cerr << "FAIL: " #c "\n"; ++failures; } } while (0)

using namespace chess;

namespace {

Position emptyPosition(Color side = Color::White) {
    Position p;
    p.sideToMove = side;
    return p;
}

void put(Position& p, const char* sq, PieceType t, Color c) { p.board.set(*squareFromString(sq), Piece{t, c}); }

std::set<std::string> movesOf(const Position& p) {
    std::set<std::string> s;
    for (const Move& m : generatePseudoLegalMoves(p)) s.insert(moveToString(m));
    return s;
}

std::set<std::string> movesFrom(const Position& p, const std::string& from) {
    std::set<std::string> s;
    for (const std::string& m : movesOf(p)) if (m.compare(0, 2, from) == 0) s.insert(m);
    return s;
}

using S = std::set<std::string>;

void testMoveGeneration() {
    const Color W = Color::White, B = Color::Black;

    // Starting position
    {
        const Position start = Position::startingPosition();
        const MoveList moves = generatePseudoLegalMoves(start);
        CHECK(moves.size() == 20);
        int pawn = 0, knight = 0;
        for (const Move& m : moves) {
            const PieceType t = start.board.at(m.from).type;
            if (t == PieceType::Pawn) ++pawn;
            if (t == PieceType::Knight) ++knight;
        }
        CHECK(pawn == 16 && knight == 4);
        Position black = start;
        black.sideToMove = B;
        CHECK(generatePseudoLegalMoves(black).size() == 20);
        for (const Move& m : generatePseudoLegalMoves(black))
            CHECK(black.board.at(m.from).color == B);
    }
    // Knight in the center, empty board
    {
        Position p = emptyPosition();
        put(p, "d4", PieceType::Knight, W);
        CHECK(movesOf(p) == (S{"d4b3", "d4b5", "d4c2", "d4c6", "d4e2", "d4e6", "d4f3", "d4f5"}));
    }
    // Knight in corner stays on board; jumps and respects friendly/enemy
    {
        Position p = emptyPosition();
        put(p, "a1", PieceType::Knight, W);
        CHECK(movesOf(p) == (S{"a1b3", "a1c2"}));
        put(p, "b3", PieceType::Pawn, W);
        put(p, "c2", PieceType::Pawn, B);
        const S m = movesFrom(p, "a1");
        CHECK(m == (S{"a1c2"}));
    }
    // Rook: open board, corner
    {
        Position p = emptyPosition();
        put(p, "a1", PieceType::Rook, W);
        CHECK(movesOf(p).size() == 14);
    }
    // Rook blocked by friend, captures enemy and stops
    {
        Position p = emptyPosition();
        put(p, "a1", PieceType::Rook, W);
        put(p, "a4", PieceType::Pawn, W);
        put(p, "d1", PieceType::Pawn, B);
        CHECK(movesFrom(p, "a1") == (S{"a1a2", "a1a3", "a1b1", "a1c1", "a1d1"}));
    }
    // Bishop: center open and blocked/capture
    {
        Position p = emptyPosition();
        put(p, "d4", PieceType::Bishop, W);
        CHECK(movesOf(p).size() == 13);
        put(p, "f6", PieceType::Pawn, B);
        put(p, "b2", PieceType::Pawn, W);
        const S m = movesFrom(p, "d4");
        CHECK(m.count("d4e5") && m.count("d4f6"));
        CHECK(!m.count("d4g7") && !m.count("d4b2") && !m.count("d4a1"));
        CHECK(m.count("d4c3"));
    }
    // Queen combines rook and bishop
    {
        Position p = emptyPosition();
        put(p, "d4", PieceType::Queen, W);
        CHECK(movesOf(p).size() == 27);
        Position c = emptyPosition();
        put(c, "h8", PieceType::Queen, W);
        CHECK(movesOf(c).size() == 21);
    }
    // King
    {
        Position p = emptyPosition();
        put(p, "e4", PieceType::King, W);
        CHECK(movesOf(p).size() == 8);
        Position c = emptyPosition();
        put(c, "h1", PieceType::King, W);
        CHECK(movesOf(c) == (S{"h1g1", "h1g2", "h1h2"}));
        put(c, "g1", PieceType::Rook, W);
        put(c, "h2", PieceType::Pawn, B);
        CHECK(movesFrom(c, "h1") == (S{"h1g2", "h1h2"}));
    }
    // Pawn: initial pushes, blocked, captures
    {
        Position p = emptyPosition();
        put(p, "e2", PieceType::Pawn, W);
        CHECK(movesOf(p) == (S{"e2e3", "e2e4"}));
        put(p, "e4", PieceType::Pawn, B);
        CHECK(movesFrom(p, "e2") == (S{"e2e3"}));
        put(p, "e3", PieceType::Knight, B);
        CHECK(movesFrom(p, "e2").empty());  // blocked directly ahead, no forward capture
        put(p, "d3", PieceType::Pawn, B);
        put(p, "f3", PieceType::Pawn, W);
        CHECK(movesFrom(p, "e2") == (S{"e2d3"}));
    }
    // Pawn not on start rank moves one square; black pawns move down
    {
        Position p = emptyPosition();
        put(p, "e3", PieceType::Pawn, W);
        CHECK(movesOf(p) == (S{"e3e4"}));
        Position b = emptyPosition(B);
        put(b, "e7", PieceType::Pawn, B);
        put(b, "d6", PieceType::Pawn, W);
        CHECK(movesOf(b) == (S{"e7e6", "e7e5", "e7d6"}));
    }
    // Pawn on edge file captures only toward the board; double push blocked at second square
    {
        Position p = emptyPosition();
        put(p, "a2", PieceType::Pawn, W);
        put(p, "b3", PieceType::Pawn, B);
        put(p, "a4", PieceType::Pawn, B);
        CHECK(movesOf(p) == (S{"a2a3", "a2b3"}));
        Position h = emptyPosition();
        put(h, "h2", PieceType::Pawn, W);
        put(h, "g3", PieceType::Pawn, B);
        CHECK(movesOf(h) == (S{"h2h3", "h2h4", "h2g3"}));
    }
    // Only the side to move generates moves
    {
        Position p = emptyPosition(B);
        put(p, "a1", PieceType::Rook, W);
        CHECK(movesOf(p).empty());
    }
    // Capture flag set
    {
        Position p = emptyPosition();
        put(p, "a1", PieceType::Rook, W);
        put(p, "a3", PieceType::Pawn, B);
        for (const Move& m : generatePseudoLegalMoves(p))
            CHECK(m.has(MoveFlagCapture) == (moveToString(m) == "a1a3"));
    }
}

std::set<std::string> legalOf(const Position& p) {
    std::set<std::string> s;
    for (const Move& m : generateLegalMoves(p)) s.insert(moveToString(m));
    return s;
}

void applyText(Position& p, const char* text) {
    const MoveParseResult r = parseMove(text);
    CHECK(r.ok());
    p.applyMove(r.move);
}

void testAttacks() {
    const Color W = Color::White, B = Color::Black;
    auto at = [](const Position& p, const char* sq, Color c) { return isSquareAttacked(p, *squareFromString(sq), c); };

    // Pawns attack diagonally forward only
    {
        Position p = emptyPosition();
        put(p, "e4", PieceType::Pawn, W);
        put(p, "e5", PieceType::Pawn, B);
        CHECK(at(p, "d5", W) && at(p, "f5", W));
        CHECK(!at(p, "e5", W) && !at(p, "d3", W) && !at(p, "f3", W));
        CHECK(at(p, "d4", B) && at(p, "f4", B));
        CHECK(!at(p, "e4", B) && !at(p, "d6", B));
        Position e = emptyPosition();
        put(e, "a4", PieceType::Pawn, W);
        CHECK(at(e, "b5", W) && !at(e, "h5", W));
    }
    // Knight
    {
        Position p = emptyPosition();
        put(p, "g1", PieceType::Knight, W);
        CHECK(at(p, "f3", W) && at(p, "h3", W) && at(p, "e2", W));
        CHECK(!at(p, "g2", W) && !at(p, "g3", W) && !at(p, "a7", W));
    }
    // Sliding: rook, bishop, queen; blocked by any piece; no wrap-around
    {
        Position p = emptyPosition();
        put(p, "a1", PieceType::Rook, W);
        put(p, "h8", PieceType::Bishop, B);
        put(p, "d4", PieceType::Queen, W);
        CHECK(at(p, "a8", W) && at(p, "h1", W) && at(p, "b2", W));
        CHECK(at(p, "a5", W) && at(p, "g4", W) && at(p, "g7", W));
        CHECK(at(p, "g7", B) && at(p, "d4", B));
        CHECK(!at(p, "c3", B));  // diagonal blocked by the queen on d4
        CHECK(!at(p, "b8", B));
        CHECK(!at(p, "b1", B));
        Position blk = emptyPosition();
        put(blk, "a1", PieceType::Rook, W);
        put(blk, "a4", PieceType::Pawn, B);
        CHECK(at(blk, "a3", W) && at(blk, "a4", W) && !at(blk, "a5", W));
        put(blk, "c1", PieceType::Knight, W);
        CHECK(at(blk, "b1", W) && at(blk, "c1", W) && !at(blk, "d1", W));
        Position wrap = emptyPosition();
        put(wrap, "h1", PieceType::Rook, W);
        CHECK(!at(wrap, "a2", W));
    }
    // King
    {
        Position p = emptyPosition();
        put(p, "e4", PieceType::King, W);
        CHECK(at(p, "d3", W) && at(p, "e5", W) && at(p, "f4", W));
        CHECK(!at(p, "e6", W) && !at(p, "e4", B));
        Position c = emptyPosition();
        put(c, "h1", PieceType::King, B);
        CHECK(at(c, "g2", B) && !at(c, "a2", B));
    }
    // isKingInCheck
    {
        Position p = emptyPosition();
        put(p, "e1", PieceType::King, W);
        CHECK(!isKingInCheck(p, W));
        put(p, "e8", PieceType::Rook, B);
        CHECK(isKingInCheck(p, W));
        put(p, "e4", PieceType::Pawn, W);
        CHECK(!isKingInCheck(p, W));
        CHECK(!isKingInCheck(p, B));  // no black king
    }
}

void testLegalMoves() {
    const Color W = Color::White, B = Color::Black;

    CHECK(generateLegalMoves(Position::startingPosition()).size() == 20);
    CHECK(getGameStatus(Position::startingPosition()) == GameStatus::Ongoing);

    // Pinned rook may only move along the pin line
    {
        Position p = emptyPosition();
        put(p, "e1", PieceType::King, W);
        put(p, "e2", PieceType::Rook, W);
        put(p, "e8", PieceType::Rook, B);
        put(p, "h8", PieceType::King, B);
        const S m = legalOf(p);
        CHECK(m.count("e2e3") && m.count("e2e8") && m.count("e2e7"));
        CHECK(!m.count("e2d2") && !m.count("e2a2"));
        // pseudo-legal still contains them
        CHECK(movesOf(p).count("e2d2"));
    }
    // King cannot step onto attacked squares
    {
        Position p = emptyPosition();
        put(p, "e1", PieceType::King, W);
        put(p, "a2", PieceType::Rook, B);
        put(p, "h8", PieceType::King, B);
        CHECK(legalOf(p) == (S{"e1d1", "e1f1"}));
    }
    // King cannot capture a protected piece
    {
        Position p = emptyPosition();
        put(p, "e1", PieceType::King, W);
        put(p, "e2", PieceType::Rook, B);
        put(p, "e8", PieceType::Rook, B);
        CHECK(!legalOf(p).count("e1e2"));
        Position q = emptyPosition();
        put(q, "e1", PieceType::King, W);
        put(q, "e2", PieceType::Rook, B);
        CHECK(legalOf(q).count("e1e2"));
    }
    // In check: capture or block or king move only
    {
        Position p = emptyPosition();
        put(p, "e1", PieceType::King, W);
        put(p, "a8", PieceType::Rook, W);
        put(p, "h4", PieceType::Rook, W);
        put(p, "b1", PieceType::Knight, W);
        put(p, "e8", PieceType::Rook, B);
        put(p, "g8", PieceType::King, B);
        CHECK(isKingInCheck(p, W));
        CHECK(legalOf(p) == (S{"e1d1", "e1f1", "e1d2", "e1f2", "a8e8", "h4e4"}));
        CHECK(getGameStatus(p) == GameStatus::Ongoing);
    }
    // Double check: only king moves
    {
        Position p = emptyPosition();
        put(p, "e1", PieceType::King, W);
        put(p, "e8", PieceType::Rook, B);
        put(p, "f3", PieceType::Knight, B);
        put(p, "a1", PieceType::Rook, W);
        put(p, "h8", PieceType::King, B);
        for (const Move& m : generateLegalMoves(p)) CHECK(m.from == *squareFromString("e1"));
    }
    // applyMove: move, capture, turn change
    {
        Position p = Position::startingPosition();
        applyText(p, "e2e4");
        CHECK(p.board.at(*squareFromString("e2")).empty());
        CHECK(p.board.at(*squareFromString("e4")) == (Piece{PieceType::Pawn, W}));
        CHECK(p.sideToMove == B);
        Position c = emptyPosition();
        put(c, "a1", PieceType::Rook, W);
        put(c, "a5", PieceType::Pawn, B);
        applyText(c, "a1a5");
        CHECK(c.board.at(*squareFromString("a5")) == (Piece{PieceType::Rook, W}));
        CHECK(c.board.at(*squareFromString("a1")).empty());
        CHECK(c.sideToMove == B);
    }
    // Checkmate: fool's mate
    {
        Position p = Position::startingPosition();
        for (const char* mv : {"f2f3", "e7e5", "g2g4", "d8h4"}) applyText(p, mv);
        CHECK(isKingInCheck(p, W));
        CHECK(generateLegalMoves(p).empty());
        CHECK(getGameStatus(p) == GameStatus::Checkmate);
    }
    // Stalemate
    {
        Position p = emptyPosition(B);
        put(p, "h8", PieceType::King, B);
        put(p, "f7", PieceType::Queen, W);
        put(p, "g6", PieceType::King, W);
        CHECK(!isKingInCheck(p, B));
        CHECK(generateLegalMoves(p).empty());
        CHECK(getGameStatus(p) == GameStatus::Stalemate);
    }
}

}  // namespace

int main() {
    testMoveGeneration();
    testAttacks();
    testLegalMoves();
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




