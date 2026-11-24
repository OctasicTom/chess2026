#include <iostream>
#include <string>
#include "chess/Notation.h"
#include "chess/Position.h"
#include "ui/ConsoleView.h"

int main() {
    const chess::Position position = chess::Position::startingPosition();
    ui::printPosition(std::cout, position);

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
