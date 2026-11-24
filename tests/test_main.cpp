#include <iostream>
#include <algorithm>
#include <set>
#include <string>
#include "../src/chess/Attacks.h"
#include <sstream>
#include "../src/chess/Fen.h"
#include "../src/chess/MoveGenerator.h"
#include "../src/chess/Perft.h"
#include "../src/players/HumanPlayer.h"
#include "../src/players/RandomComputerPlayer.h"
#include "../src/ui/GameLoop.h"
#include "../src/ui/PlayerSetup.h"
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

Position fromFen(const char* fen) {
    const auto p = parseFen(fen);
    CHECK(p.has_value());
    return p ? *p : Position{};
}

bool hasMove(const Position& p, const char* text) { return legalOf(p).count(text) != 0; }

const Move* findMove(const MoveList& list, const char* text) {
    for (const Move& m : list) if (moveToString(m) == text) return &m;
    return nullptr;
}

void testFen() {
    CHECK(toFen(Position::startingPosition()) == kStartingFen);
    Position s = fromFen(kStartingFen);
    CHECK(s.sideToMove == Color::White);
    CHECK(s.castlingRights == (CastleWhiteKing | CastleWhiteQueen | CastleBlackKing | CastleBlackQueen));
    CHECK(s.enPassantSquare == kNoSquare);
    CHECK(s.halfmoveClock == 0 && s.fullmoveNumber == 1);
    CHECK(s.board.at(*squareFromString("e1")) == (Piece{PieceType::King, Color::White}));
    CHECK(s.board.at(*squareFromString("d8")) == (Piece{PieceType::Queen, Color::Black}));

    const char* e4 = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
    Position a = fromFen(e4);
    CHECK(a.sideToMove == Color::Black);
    CHECK(a.enPassantSquare == *squareFromString("e3"));
    CHECK(toFen(a) == e4);

    Position b = fromFen("r3k2r/8/8/8/8/8/8/R3K2R w Kq - 37 85");
    CHECK(b.castlingRights == (CastleWhiteKing | CastleBlackQueen));
    CHECK(b.halfmoveClock == 37 && b.fullmoveNumber == 85);
    CHECK(toFen(b) == "r3k2r/8/8/8/8/8/8/R3K2R w Kq - 37 85");

    Position none = fromFen("4k3/8/8/8/8/8/8/4K3 w - - 0 1");
    CHECK(none.castlingRights == 0);

    // Counters, castling and en passant evolve correctly while playing
    Position g = Position::startingPosition();
    applyText(g, "e2e4");
    CHECK(toFen(g) == e4);
    applyText(g, "c7c5");
    CHECK(toFen(g) == "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2");
    applyText(g, "g1f3");
    CHECK(toFen(g) == "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");

    // Short (4-field) FEN and rejects
    CHECK(parseFen("4k3/8/8/8/8/8/8/4K3 w - -").has_value());
    CHECK(!parseFen(""));
    CHECK(!parseFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP w KQkq - 0 1"));
    CHECK(!parseFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBN w KQkq - 0 1"));
    CHECK(!parseFen("rnbqkbnr/pppppppp/9/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
    CHECK(!parseFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNX w KQkq - 0 1"));
    CHECK(!parseFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR x KQkq - 0 1"));
    CHECK(!parseFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkz - 0 1"));
    CHECK(!parseFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq e4 0 1"));
    CHECK(!parseFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - x 1"));
    CHECK(!parseFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0"));
}

void testSpecialMoves() {
    const char* both = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";

    // Castling
    {
        Position p = fromFen(both);
        CHECK(hasMove(p, "e1g1") && hasMove(p, "e1c1"));
        const MoveList moves = generateLegalMoves(p);
        CHECK(findMove(moves, "e1g1")->has(MoveFlagCastleKing));
        CHECK(findMove(moves, "e1c1")->has(MoveFlagCastleQueen));
        Position k = p; applyText(k, "e1g1");
        CHECK(toFen(k) == "r3k2r/8/8/8/8/8/8/R4RK1 b kq - 1 1");
        Position q = p; applyText(q, "e1c1");
        CHECK(toFen(q) == "r3k2r/8/8/8/8/8/8/2KR3R b kq - 1 1");
        Position bk = fromFen("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1");
        CHECK(hasMove(bk, "e8g8") && hasMove(bk, "e8c8"));
        applyText(bk, "e8c8");
        CHECK(toFen(bk) == "2kr3r/8/8/8/8/8/8/R3K2R w KQ - 1 2");
        applyText(bk, "e1g1");  // sanity: applyMove infers castling without flags
        CHECK(toFen(bk) == "2kr3r/8/8/8/8/8/8/R4RK1 b - - 2 2");
    }
    // Castling in check, through check, onto check, with blocked squares
    {
        Position inCheck = fromFen("4r1k1/8/8/8/8/8/8/R3K2R w KQ - 0 1");
        CHECK(!hasMove(inCheck, "e1g1") && !hasMove(inCheck, "e1c1"));
        Position throughK = fromFen("5rk1/8/8/8/8/8/8/R3K2R w KQ - 0 1");
        CHECK(!hasMove(throughK, "e1g1") && hasMove(throughK, "e1c1"));
        Position throughQ = fromFen("3r2k1/8/8/8/8/8/8/R3K2R w KQ - 0 1");
        CHECK(!hasMove(throughQ, "e1c1") && hasMove(throughQ, "e1g1"));
        Position ontoK = fromFen("6rk/8/8/8/8/8/8/R3K2R w KQ - 0 1");
        CHECK(!hasMove(ontoK, "e1g1") && hasMove(ontoK, "e1c1"));
        Position bAttacked = fromFen("1r4k1/8/8/8/8/8/8/R3K2R w KQ - 0 1");  // b1 attacked: queenside still legal
        CHECK(hasMove(bAttacked, "e1c1"));
        Position blocked = fromFen("r3k2r/8/8/8/8/8/8/RN2KB1R w KQkq - 0 1");
        CHECK(!hasMove(blocked, "e1g1") && !hasMove(blocked, "e1c1"));
        Position noRights = fromFen("r3k2r/8/8/8/8/8/8/R3K2R w - - 0 1");
        CHECK(!hasMove(noRights, "e1g1") && !hasMove(noRights, "e1c1"));
    }
    // Rights lost after king move / rook move / rook capture
    {
        Position p = fromFen(both);
        for (const char* mv : {"e1d1", "e8d8", "d1e1", "d8e8"}) applyText(p, mv);
        CHECK(p.castlingRights == 0);
        CHECK(!hasMove(p, "e1g1") && !hasMove(p, "e1c1"));

        Position r = fromFen(both);
        applyText(r, "a1b1"); applyText(r, "a8b8"); applyText(r, "b1a1"); applyText(r, "b8a8");
        CHECK(!hasMove(r, "e1c1") && hasMove(r, "e1g1"));
        CHECK(r.castlingRights == (CastleWhiteKing | CastleBlackKing));

        Position c = fromFen("r3k2r/8/8/8/8/8/6b1/R3K2R b KQkq - 0 1");
        applyText(c, "g2h1");
        CHECK(!(c.castlingRights & CastleWhiteKing) && (c.castlingRights & CastleWhiteQueen));
        CHECK(!hasMove(c, "e1g1"));
        Position c2 = fromFen("r3k2r/1B6/8/8/8/8/8/R3K2R w KQkq - 0 1");
        applyText(c2, "b7a8");
        CHECK(!(c2.castlingRights & CastleBlackQueen) && (c2.castlingRights & CastleBlackKing));
    }
    // En passant
    {
        Position p = fromFen("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1");
        const MoveList moves = generateLegalMoves(p);
        const Move* ep = findMove(moves, "e5d6");
        CHECK(ep && ep->has(MoveFlagEnPassant) && ep->has(MoveFlagCapture));
        applyText(p, "e5d6");
        CHECK(toFen(p) == "4k3/8/3P4/8/8/8/8/4K3 b - - 0 1");

        Position d = fromFen("4k3/3p4/8/4P3/8/8/8/4K3 b - - 0 1");
        applyText(d, "d7d5");
        CHECK(d.enPassantSquare == *squareFromString("d6"));
        CHECK(hasMove(d, "e5d6"));
        applyText(d, "e1e2");   // white declines
        CHECK(d.enPassantSquare == kNoSquare);
        applyText(d, "e8e7");
        CHECK(!hasMove(d, "e5d6"));

        // Black capturing en passant
        Position bl = fromFen("4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1");
        CHECK(hasMove(bl, "d4e3"));
        applyText(bl, "d4e3");
        CHECK(toFen(bl) == "4k3/8/8/8/8/4p3/8/4K3 w - - 0 2");

        // Exposes own king along the rank: illegal
        Position x = fromFen("8/8/8/K2pP2r/8/8/8/4k3 w - d6 0 1");
        CHECK(movesOf(x).count("e5d6"));
        CHECK(!hasMove(x, "e5d6") && hasMove(x, "e5e6"));

        // No ep move unless a pawn is really there / wrong side pawns
        Position none = fromFen("4k3/8/8/4P3/8/8/8/4K3 w - d6 0 1");
        CHECK(!hasMove(none, "e5d6"));
    }
    // Promotion
    {
        Position p = fromFen("8/P6k/8/8/8/8/8/K7 w - - 0 1");
        const S m = legalOf(p);
        CHECK(m.count("a7a8q") && m.count("a7a8r") && m.count("a7a8b") && m.count("a7a8n"));
        CHECK(!m.count("a7a8"));
        CHECK(movesFrom(p, "a7").size() == 4);
        const MoveList list = generateLegalMoves(p);
        CHECK(findMove(list, "a7a8n")->promotion == PieceType::Knight);
        CHECK(findMove(list, "a7a8n")->has(MoveFlagPromotion));
        Position n = p; applyText(n, "a7a8n");
        CHECK(n.board.at(*squareFromString("a8")) == (Piece{PieceType::Knight, Color::White}));
        Position r = p; applyText(r, "a7a8r");
        CHECK(toFen(r) == "R7/7k/8/8/8/8/8/K7 b - - 0 1");

        Position cap = fromFen("1n5k/P7/8/8/8/8/8/K7 w - - 0 1");
        const S c = legalOf(cap);
        CHECK(c.count("a7b8q") && c.count("a7b8r") && c.count("a7b8b") && c.count("a7b8n"));
        CHECK(c.count("a7a8q") && c.size() >= 8);
        const MoveList capMoves = generateLegalMoves(cap);
        const Move* mv = findMove(capMoves, "a7b8q");
        CHECK(mv && mv->has(MoveFlagCapture) && mv->has(MoveFlagPromotion));
        applyText(cap, "a7b8q");
        CHECK(toFen(cap) == "1Q5k/8/8/8/8/8/8/K7 b - - 0 1");

        Position black = fromFen("k7/8/8/8/8/8/p7/7K b - - 0 1");
        CHECK(legalOf(black) == (S{"a8a7", "a8b8", "a8b7", "a2a1q", "a2a1r", "a2a1b", "a2a1n"}));
    }
}

void testPerft() {
    const Position start = Position::startingPosition();
    CHECK(perft(start, 1) == 20);
    CHECK(perft(start, 2) == 400);
    CHECK(perft(start, 3) == 8902);
    CHECK(perft(start, 4) == 197281);

    const Position kiwi = fromFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    CHECK(perft(kiwi, 1) == 48 && perft(kiwi, 2) == 2039 && perft(kiwi, 3) == 97862);
    const Position p3 = fromFen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    CHECK(perft(p3, 1) == 14 && perft(p3, 2) == 191 && perft(p3, 3) == 2812 && perft(p3, 4) == 43238);
    const Position p4 = fromFen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    CHECK(perft(p4, 1) == 6 && perft(p4, 2) == 264 && perft(p4, 3) == 9467);
    const Position p5 = fromFen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    CHECK(perft(p5, 1) == 44 && perft(p5, 2) == 1486 && perft(p5, 3) == 62379);
}

// Plays back a fixed list of coordinate moves, to show the loop treats every Player alike.
class ScriptedPlayer : public players::Player {
public:
    explicit ScriptedPlayer(std::vector<std::string> moves) : moves_(std::move(moves)) {}
    Move chooseMove(const Position&, const std::vector<Move>& legal) override {
        if (next_ >= moves_.size()) return Move{};
        for (const Move& m : legal) if (moveToString(m) == moves_[next_]) { ++next_; return m; }
        return Move{1, 1, PieceType::None, 0};  // deliberately not legal
    }
private:
    std::vector<std::string> moves_;
    std::size_t next_ = 0;
};

ui::GameResult play(players::Player& w, players::Player& b, std::string* output = nullptr,
                    int maxPlies = 0, Position start = Position::startingPosition()) {
    std::ostringstream out;
    ui::GameOptions opt;
    opt.maxPlies = maxPlies;
    const ui::GameResult r = ui::runGame(w, b, out, start, opt);
    if (output) *output = out.str();
    return r;
}

void testPlayers() {
    // Human vs Human (scripted input, with invalid and illegal entries along the way)
    {
        std::istringstream in("f2f3\nzz\ne2e5\ne7e8\ne7e5\ne7e5\ng2g4\nd8h4\n");
        std::ostringstream out;
        players::HumanPlayer w(in, out), b(in, out);
        ui::GameResult r = ui::runGame(w, b, out, Position::startingPosition());
        const std::string s = out.str();
        CHECK(r.outcome == ui::GameOutcome::Checkmate && r.winner == Color::Black && r.plies == 4);
        CHECK(s.find("Invalid input") != std::string::npos);
        CHECK(s.find("Illegal move: e2e5") != std::string::npos);
        CHECK(s.find("Checkmate! Black wins.") != std::string::npos);
        CHECK(s.find("White plays f2f3") != std::string::npos);
    }
    {
        std::istringstream in("quit\n");
        std::ostringstream out;
        players::HumanPlayer h(in, out);
        ui::GameResult r = ui::runGame(h, h, out, fromFen("4r1k1/8/8/8/8/8/8/4K3 w - - 0 1"));
        CHECK(out.str().find("White is in check.") != std::string::npos);
        CHECK(r.outcome == ui::GameOutcome::Abandoned && r.plies == 0);
    }
    {
        std::istringstream in("a7a8\na7a8q\nquit\n");
        std::ostringstream out;
        players::HumanPlayer h(in, out);
        ui::GameResult r = ui::runGame(h, h, out, fromFen("8/P6k/8/8/8/8/8/K7 w - - 0 1"));
        CHECK(out.str().find("Promotion piece required") != std::string::npos);
        CHECK(r.plies == 1 && r.outcome == ui::GameOutcome::Abandoned);  // promoted, then quit as Black
    }
    {
        std::istringstream in("");  // end of input behaves like quit
        std::ostringstream out;
        players::HumanPlayer h(in, out);
        CHECK(play(h, h, nullptr, 0, fromFen("4k3/8/8/8/8/8/8/4K3 w - - 0 1")).outcome ==
              ui::GameOutcome::Abandoned);
        std::istringstream in3("");
        std::ostringstream out3;
        players::HumanPlayer h3(in3, out3);
        std::ostringstream o;
        CHECK(ui::runGame(h3, h3, o, fromFen("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1")).outcome ==
              ui::GameOutcome::Stalemate);
    }
    // Scripted players: same loop, no console
    {
        ScriptedPlayer w({"f2f3", "g2g4"}), b({"e7e5", "d8h4"});
        const ui::GameResult r = play(w, b);
        CHECK(r.outcome == ui::GameOutcome::Checkmate && r.winner == Color::Black);
        ScriptedPlayer bad({"e2e5"}), idle({});
        CHECK(play(bad, idle).outcome == ui::GameOutcome::IllegalMove);
    }
    // RandomComputerPlayer: legal, deterministic with a seed, independent of console
    {
        const Position start = Position::startingPosition();
        const MoveList legal = generateLegalMoves(start);
        players::RandomComputerPlayer a(42), b(42), c(7);
        bool differs = false;
        for (int i = 0; i < 50; ++i) {
            const Move ma = a.chooseMove(start, legal), mb = b.chooseMove(start, legal), mc = c.chooseMove(start, legal);
            CHECK(findMove(legal, moveToString(ma).c_str()) != nullptr);
            CHECK(moveToString(ma) == moveToString(mb));
            if (moveToString(ma) != moveToString(mc)) differs = true;
        }
        CHECK(differs);
        CHECK(a.chooseMove(start, {}).isNull());
        players::RandomComputerPlayer unseeded;
        CHECK(findMove(legal, moveToString(unseeded.chooseMove(start, legal)).c_str()) != nullptr);
    }
    // Human vs Computer and Computer vs Human
    {
        std::istringstream in("e2e4\nquit\n");
        std::ostringstream out;
        players::HumanPlayer h(in, out);
        players::RandomComputerPlayer cpu(1);
        ui::GameResult r = ui::runGame(h, cpu, out, Position::startingPosition());
        CHECK(r.plies == 2 && r.outcome == ui::GameOutcome::Abandoned);
        CHECK(out.str().find("Black plays ") != std::string::npos);
        CHECK(r.finalPosition.board.at(*squareFromString("e4")) == (Piece{PieceType::Pawn, Color::White}));
    }
    {
        std::istringstream in("quit\n");
        std::ostringstream out;
        players::HumanPlayer h(in, out);
        players::RandomComputerPlayer cpu(1);
        ui::GameResult r = ui::runGame(cpu, h, out, Position::startingPosition());
        CHECK(r.plies == 1 && r.outcome == ui::GameOutcome::Abandoned);
        CHECK(out.str().find("White plays ") != std::string::npos);
        CHECK(out.str().find("Enter move for Black") != std::string::npos);
        CHECK(out.str().find("Enter move for White") == std::string::npos);
    }
    // Computer vs Computer: complete games with no input, bounded by a ply limit
    {
        int mate = 0, stale = 0, limit = 0, longest = 0;
        for (std::uint32_t seed = 1; seed <= 25; ++seed) {
            players::RandomComputerPlayer w(seed), b(seed + 1000);
            const ui::GameResult r = play(w, b, nullptr, 1000);
            CHECK(r.outcome == ui::GameOutcome::Checkmate || r.outcome == ui::GameOutcome::Stalemate ||
                  r.outcome == ui::GameOutcome::PlyLimit);
            CHECK(r.plies <= 1000);
            if (r.outcome == ui::GameOutcome::Checkmate) ++mate;
            else if (r.outcome == ui::GameOutcome::Stalemate) ++stale;
            else ++limit;
            if (r.plies > longest) longest = r.plies;
        }
        std::cout << "Computer-vs-Computer, 25 seeded games: " << mate << " checkmates, " << stale
                  << " stalemates, " << limit << " hit the 1000-ply limit (longest " << longest << " plies)\n";
        CHECK(mate + stale + limit == 25);

        // Reproducible with the same seeds
        players::RandomComputerPlayer w1(5), b1(6), w2(5), b2(6);
        const ui::GameResult r1 = play(w1, b1, nullptr, 1000), r2 = play(w2, b2, nullptr, 1000);
        CHECK(r1.plies == r2.plies && toFen(r1.finalPosition) == toFen(r2.finalPosition));

        // The ply limit really stops a game
        players::RandomComputerPlayer w3(5), b3(6);
        const ui::GameResult lim = play(w3, b3, nullptr, 10);
        CHECK(lim.plies <= 10);
    }
    // Menu validation
    {
        std::istringstream in("x\n3\n\n2\n 1 \n");
        std::ostringstream out;
        CHECK(ui::promptPlayerKind(in, out, "White") == ui::PlayerKind::Computer);
        CHECK(out.str().find("Please enter 1 or 2.") != std::string::npos);
        CHECK(ui::promptPlayerKind(in, out, "Black") == ui::PlayerKind::Human);
        CHECK(!ui::promptPlayerKind(in, out, "Black").has_value());
    }
}
}  // namespace

int main() {
    testMoveGeneration();
    testAttacks();
    testLegalMoves();
    testFen();
    testSpecialMoves();
    testPerft();
    testPlayers();
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









