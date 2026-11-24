#include <iostream>
#include "chess/Position.h"
#include "ui/ConsoleView.h"

int main() {
    const chess::Position position = chess::Position::startingPosition();
    ui::printPosition(std::cout, position);
    return 0;
}
