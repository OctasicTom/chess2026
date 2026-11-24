#include "GameLoop.h"
#include <algorithm>
#include <cctype>
#include <istream>
#include <ostream>
#include <string>
#include "../chess/Attacks.h"
#include "../chess/MoveGenerator.h"
#include "../chess/Notation.h"
#include "ConsoleView.h"

namespace ui {

namespace {

const char* colorName(chess::Color c) { return c == chess::Color::White ? "White" : "Black"; }

bool isQuit(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    const auto b = text.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return false;
    const auto e = text.find_last_not_of(" \t\r\n");
    text = text.substr(b, e - b + 1);
    return text == "quit";
}

}  // namespace

void runHumanVsHuman(std::istream& in, std::ostream& out, chess::Position position) {
    while (true) {
        out << '\n';
        printPosition(out, position);

        const chess::GameStatus status = chess::getGameStatus(position);
        if (status == chess::GameStatus::Checkmate) {
            out << "Checkmate! " << colorName(chess::opposite(position.sideToMove)) << " wins.\n";
            return;
        }
        if (status == chess::GameStatus::Stalemate) {
            out << "Stalemate! The game is a draw.\n";
            return;
        }
        if (chess::isKingInCheck(position, position.sideToMove))
            out << colorName(position.sideToMove) << " is in check.\n";

        const chess::MoveList legal = chess::generateLegalMoves(position);
        while (true) {
            out << "Enter move for " << colorName(position.sideToMove) << " (e.g. e2e4, e7e8q, or quit): ";
            std::string line;
            if (!std::getline(in, line)) {
                out << "\nEnd of input. Game abandoned.\n";
                return;
            }
            if (isQuit(line)) {
                out << "Game abandoned.\n";
                return;
            }

            const chess::MoveParseResult parsed = chess::parseMove(line);
            if (!parsed.ok()) {
                out << "Invalid input: " << chess::describe(parsed.error) << ".\n";
                continue;
            }
            const auto match = std::find_if(legal.begin(), legal.end(), [&](const chess::Move& m) {
                return m.from == parsed.move.from && m.to == parsed.move.to &&
                       m.promotion == parsed.move.promotion;
            });
            if (match == legal.end()) {
                const bool needsPromotion = parsed.move.promotion == chess::PieceType::None &&
                    std::any_of(legal.begin(), legal.end(), [&](const chess::Move& m) {
                        return m.from == parsed.move.from && m.to == parsed.move.to;
                    });
                if (needsPromotion)
                    out << "Promotion piece required: add q, r, b or n (e.g. "
                        << chess::moveToString(parsed.move) << "q).\n";
                else
                    out << "Illegal move: " << chess::moveToString(parsed.move) << ".\n";
                continue;
            }
            position.applyMove(*match);
            break;
        }
    }
}

}  // namespace ui
