#include "PlayerSetup.h"
#include <istream>
#include <ostream>
#include <string>
#include "../players/HumanPlayer.h"
#include "../players/RandomComputerPlayer.h"

namespace ui {

std::optional<PlayerKind> promptPlayerKind(std::istream& in, std::ostream& out, const char* colorName) {
    while (true) {
        out << "\n" << colorName << " player:\n  1. Human\n  2. Computer\nChoice: ";
        std::string line;
        if (!std::getline(in, line)) return std::nullopt;
        const auto b = line.find_first_not_of(" \t\r\n");
        const auto e = line.find_last_not_of(" \t\r\n");
        const std::string choice = b == std::string::npos ? std::string() : line.substr(b, e - b + 1);
        if (choice == "1") return PlayerKind::Human;
        if (choice == "2") return PlayerKind::Computer;
        out << "Please enter 1 or 2.\n";
    }
}

std::unique_ptr<players::Player> makePlayer(PlayerKind kind, std::istream& in, std::ostream& out,
                                            std::optional<std::uint32_t> seed) {
    if (kind == PlayerKind::Human) return std::make_unique<players::HumanPlayer>(in, out);
    if (seed) return std::make_unique<players::RandomComputerPlayer>(*seed);
    return std::make_unique<players::RandomComputerPlayer>();
}

}  // namespace ui
