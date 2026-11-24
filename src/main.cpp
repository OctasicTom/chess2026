#include <iostream>
#include <string>
#include "chess/Attacks.h"
#include "chess/MoveGenerator.h"
#include "chess/Notation.h"
#include "chess/Position.h"
#include "ui/ConsoleView.h"

int main() {
    const chess::Position position = chess::Position::startingPosition();
    ui::printPosition(std::cout, position);

    const chess::MoveList moves = chess::generateLegalMoves(position);
    std::cout << "\nSide to move in check: "
              << (chess::isKingInCheck(position, position.sideToMove) ? "yes" : "no") << "\n";
    std::cout << "Legal moves:\n";
    for (const chess::Move& mv : moves) std::cout << chess::moveToString(mv) << ' ';
    std::cout << "\nTotal: " << moves.size() << " legal moves\n";
    const chess::GameStatus status = chess::getGameStatus(position);
    if (status != chess::GameStatus::Ongoing) std::cout << chess::describe(status) << "\n";

    std::cout << "\nEnter a move (e.g. e2e4, e7e8q): ";
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cout << "\nNo input.\n";
        return 0;
    }

    const chess::MoveParseResult result = chess::parseMove(line);
    if (!result.ok()) {
        std::cout << "Invalid move: " << chess::describe(result.error) << '\n';
        return 1;
    }
    std::cout << "Source:      " << chess::squareToString(result.move.from) << " (" << int(result.move.from) << ")\n"
              << "Destination: " << chess::squareToString(result.move.to) << " (" << int(result.move.to) << ")\n";
    if (result.move.promotion != chess::PieceType::None)
        std::cout << "Promotion:   " << chess::Piece{result.move.promotion, chess::Color::White}.toChar() << '\n';
    return 0;
}


