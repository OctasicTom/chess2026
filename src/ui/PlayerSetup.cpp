#include "PlayerSetup.h"
#include <istream>
#include <ostream>
#include <string>
#include "../players/ComputerPlayer.h"
#include "../players/HumanPlayer.h"
#include "../players/RandomComputerPlayer.h"

namespace ui {

namespace {

std::string trimmed(const std::string& line) {
    const auto b = line.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return std::string();
    return line.substr(b, line.find_last_not_of(" \t\r\n") - b + 1);
}

}  // namespace

std::optional<PlayerKind> promptPlayerKind(std::istream& in, std::ostream& out, const char* colorName) {
    while (true) {
        out << "\n" << colorName << " player:\n  1. Human\n  2. Random Computer\n  3. Search Computer (alpha-beta, positional evaluation)\n  4. Basic Search Computer (plain negamax, material only)\nChoice: ";
        std::string line;
        if (!std::getline(in, line)) return std::nullopt;
        const std::string choice = trimmed(line);
        if (choice == "1") return PlayerKind::Human;
        if (choice == "2") return PlayerKind::RandomComputer;
        if (choice == "3") return PlayerKind::SearchComputer;
        if (choice == "4") return PlayerKind::BasicSearchComputer;
        out << "Please enter 1, 2, 3 or 4.\n";
    }
}

std::optional<int> promptSearchDepth(std::istream& in, std::ostream& out) {
    while (true) {
        out << "Search depth (1-" << kMaxSearchDepth << ", default " << kDefaultSearchDepth << "): ";
        std::string line;
        if (!std::getline(in, line)) return std::nullopt;
        const std::string choice = trimmed(line);
        if (choice.empty()) return kDefaultSearchDepth;
        if (choice.size() == 1 && choice[0] >= '1' && choice[0] <= '0' + kMaxSearchDepth)
            return choice[0] - '0';
        out << "Please enter a number from 1 to " << kMaxSearchDepth << ".\n";
    }
}

std::unique_ptr<players::Player> makePlayer(PlayerKind kind, std::istream& in, std::ostream& out,
                                            std::optional<std::uint32_t> seed, int searchDepth) {
    switch (kind) {
        case PlayerKind::Human: return std::make_unique<players::HumanPlayer>(in, out);
        case PlayerKind::SearchComputer:
            return std::make_unique<players::ComputerPlayer>(engine::advancedConfig(searchDepth), &out);
        case PlayerKind::BasicSearchComputer:
            return std::make_unique<players::ComputerPlayer>(engine::basicConfig(searchDepth), &out);
        case PlayerKind::RandomComputer: break;
    }
    if (seed) return std::make_unique<players::RandomComputerPlayer>(*seed);
    return std::make_unique<players::RandomComputerPlayer>();
}

}  // namespace ui

