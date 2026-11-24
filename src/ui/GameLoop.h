#pragma once
#include <iosfwd>
#include "../chess/Position.h"
#include "../players/Player.h"

namespace ui {

enum class GameOutcome {
    Checkmate,    // GameResult::winner is set
    Stalemate,
    Abandoned,    // a player returned a null move (quit / end of input)
    PlyLimit,     // stopped after GameOptions::maxPlies (draw rules are not implemented yet)
    IllegalMove   // a player returned a move that is not legal (player bug)
};

struct GameOptions {
    int maxPlies = 0;  // 0 = unlimited
};

struct GameResult {
    GameOutcome outcome = GameOutcome::Abandoned;
    chess::Color winner = chess::Color::None;
    int plies = 0;  // half-moves played
    chess::Position finalPosition;
};

// Plays a game from `start`, asking whichever Player owns the side to move for each move.
// Humans and computers are treated identically.
GameResult runGame(players::Player& white, players::Player& black, std::ostream& out,
                   chess::Position start, const GameOptions& options = GameOptions());

}  // namespace ui
