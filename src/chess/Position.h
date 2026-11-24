#pragma once
#include "Board.h"

namespace chess {

enum CastlingRight : std::uint8_t {
    CastleWhiteKing = 1, CastleWhiteQueen = 2, CastleBlackKing = 4, CastleBlackQueen = 8
};

// A full game state: board plus side to move, castling, en passant, clocks.
struct Position {
    Board board;
    Color sideToMove = Color::White;
    std::uint8_t castlingRights = 0;
    Square enPassantSquare = kNoSquare;
    int halfmoveClock = 0;
    int fullmoveNumber = 1;

    static Position startingPosition();

    // Applies a normal move (including normal captures) and passes the turn. Does not check
    // legality. Castling, en passant and promotion are not handled yet.
    void applyMove(const Move& move);
};

}  // namespace chess

