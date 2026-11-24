#include "HumanPlayer.h"
#include <algorithm>
#include <cctype>
#include <istream>
#include <ostream>
#include <string>
#include "../chess/Notation.h"

namespace players {

namespace {

bool isQuit(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    const auto b = text.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return false;
    const auto e = text.find_last_not_of(" \t\r\n");
    return text.substr(b, e - b + 1) == "quit";
}

}  // namespace

chess::Move HumanPlayer::chooseMove(const chess::Position& position,
                                    const std::vector<chess::Move>& legalMoves) {
    const char* side = position.sideToMove == chess::Color::White ? "White" : "Black";
    while (true) {
        out_ << "Enter move for " << side << " (e.g. e2e4, e7e8q, or quit): ";
        std::string line;
        if (!std::getline(in_, line)) return chess::Move{};
        if (isQuit(line)) return chess::Move{};

        const chess::MoveParseResult parsed = chess::parseMove(line);
        if (!parsed.ok()) {
            out_ << "Invalid input: " << chess::describe(parsed.error) << ".\n";
            continue;
        }
        const auto match = std::find_if(legalMoves.begin(), legalMoves.end(), [&](const chess::Move& m) {
            return m.from == parsed.move.from && m.to == parsed.move.to &&
                   m.promotion == parsed.move.promotion;
        });
        if (match != legalMoves.end()) return *match;

        const bool needsPromotion = parsed.move.promotion == chess::PieceType::None &&
            std::any_of(legalMoves.begin(), legalMoves.end(), [&](const chess::Move& m) {
                return m.from == parsed.move.from && m.to == parsed.move.to;
            });
        if (needsPromotion)
            out_ << "Promotion piece required: add q, r, b or n (e.g. "
                 << chess::moveToString(parsed.move) << "q).\n";
        else
            out_ << "Illegal move: " << chess::moveToString(parsed.move) << ".\n";
    }
}

}  // namespace players
