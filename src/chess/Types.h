#pragma once
#include <cstdint>

namespace chess {

enum class Color : std::uint8_t { White, Black, None };

enum class PieceType : std::uint8_t { None, Pawn, Knight, Bishop, Rook, Queen, King };

constexpr Color opposite(Color c) {
    return c == Color::White ? Color::Black : (c == Color::Black ? Color::White : Color::None);
}

struct Piece {
    PieceType type = PieceType::None;
    Color color = Color::None;

    constexpr bool empty() const { return type == PieceType::None; }
    constexpr bool operator==(const Piece& o) const { return type == o.type && color == o.color; }
    constexpr bool operator!=(const Piece& o) const { return !(*this == o); }

    char toChar() const;  // FEN letter: uppercase = white, '.' = empty
};

// Squares are indexed 0..63: a1 = 0, b1 = 1, ... h1 = 7, a2 = 8, ... h8 = 63.
using Square = std::uint8_t;
constexpr int kNumSquares = 64;
constexpr Square kNoSquare = 64;

constexpr int fileOf(Square s) { return s & 7; }
constexpr int rankOf(Square s) { return s >> 3; }
constexpr Square makeSquare(int file, int rank) { return static_cast<Square>(rank * 8 + file); }

enum MoveFlag : std::uint8_t {
    MoveFlagNone       = 0,
    MoveFlagCapture    = 1 << 0,
    MoveFlagDoublePush = 1 << 1,
    MoveFlagEnPassant  = 1 << 2,
    MoveFlagCastleKing = 1 << 3,
    MoveFlagCastleQueen = 1 << 4,
    MoveFlagPromotion  = 1 << 5
};

struct Move {
    Square from = kNoSquare;
    Square to = kNoSquare;
    PieceType promotion = PieceType::None;
    std::uint8_t flags = MoveFlagNone;

    constexpr bool isNull() const { return from == kNoSquare; }
    constexpr bool has(MoveFlag f) const { return (flags & f) != 0; }
};

}  // namespace chess
