#include "GameLoop.h"
#include <algorithm>
#include <ostream>
#include "../chess/Attacks.h"
#include "../chess/MoveGenerator.h"
#include "../chess/Notation.h"
#include "ConsoleView.h"

namespace ui {

namespace {

const char* colorName(chess::Color c) { return c == chess::Color::White ? "White" : "Black"; }

}  // namespace

GameResult runGame(players::Player& white, players::Player& black, std::ostream& out,
                   chess::Position position, const GameOptions& options) {
    GameResult result;
    while (true) {
        out << '\n';
        printPosition(out, position);
        result.finalPosition = position;

        const chess::GameStatus status = chess::getGameStatus(position);
        if (status == chess::GameStatus::Checkmate) {
            result.outcome = GameOutcome::Checkmate;
            result.winner = chess::opposite(position.sideToMove);
            out << "Checkmate! " << colorName(result.winner) << " wins.\n";
            return result;
        }
        if (status == chess::GameStatus::Stalemate) {
            result.outcome = GameOutcome::Stalemate;
            out << "Stalemate! The game is a draw.\n";
            return result;
        }
        if (options.maxPlies > 0 && result.plies >= options.maxPlies) {
            result.outcome = GameOutcome::PlyLimit;
            out << "Game stopped after " << result.plies << " plies (move limit reached).\n";
            return result;
        }
        if (chess::isKingInCheck(position, position.sideToMove))
            out << colorName(position.sideToMove) << " is in check.\n";

        const chess::MoveList legal = chess::generateLegalMoves(position);
        players::Player& current = position.sideToMove == chess::Color::White ? white : black;
        const chess::Move move = current.chooseMove(position, legal);

        if (move.isNull()) {
            result.outcome = GameOutcome::Abandoned;
            out << "Game abandoned.\n";
            return result;
        }
        const bool isLegal = std::any_of(legal.begin(), legal.end(), [&](const chess::Move& m) {
            return m.from == move.from && m.to == move.to && m.promotion == move.promotion;
        });
        if (!isLegal) {
            result.outcome = GameOutcome::IllegalMove;
            out << "Player returned an illegal move: " << chess::moveToString(move) << ".\n";
            return result;
        }

        out << colorName(position.sideToMove) << " plays " << chess::moveToString(move) << '\n';
        position.applyMove(move);
        ++result.plies;
    }
}

}  // namespace ui
