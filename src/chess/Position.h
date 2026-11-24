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

    // Applies a move and passes the turn, updating castling rights, en passant target and clocks.\n    // Handles captures, castling, en passant and promotion. Does not check legality.
    void applyMove(const Move& move);
};

}  // namespace chess


