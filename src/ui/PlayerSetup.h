#pragma once
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <optional>
#include "../players/Player.h"

namespace ui {

enum class PlayerKind { Human, Computer };

// Asks "1. Human / 2. Computer" until a valid choice is entered. Returns nullopt at end of input.
std::optional<PlayerKind> promptPlayerKind(std::istream& in, std::ostream& out, const char* colorName);

// Human players read from `in`/`out`. Computer players use `seed` if given, else a random seed.
std::unique_ptr<players::Player> makePlayer(PlayerKind kind, std::istream& in, std::ostream& out,
                                            std::optional<std::uint32_t> seed = std::nullopt);

}  // namespace ui
