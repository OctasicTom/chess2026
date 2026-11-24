#pragma once
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <optional>
#include "../players/Player.h"

namespace ui {

enum class PlayerKind { Human, RandomComputer, SearchComputer };

constexpr int kDefaultSearchDepth = 3;
constexpr int kMaxSearchDepth = 6;

// Asks for 1 (Human), 2 (Random Computer) or 3 (Search Computer) until valid. nullopt at end of input.
std::optional<PlayerKind> promptPlayerKind(std::istream& in, std::ostream& out, const char* colorName);

// Asks for a search depth in 1..kMaxSearchDepth; an empty answer selects the default.
std::optional<int> promptSearchDepth(std::istream& in, std::ostream& out);

// Human players read from `in`/`out`. Random computers use `seed` if given. Search computers use
// `searchDepth` and print diagnostics to `out`.
std::unique_ptr<players::Player> makePlayer(PlayerKind kind, std::istream& in, std::ostream& out,
                                            std::optional<std::uint32_t> seed = std::nullopt,
                                            int searchDepth = kDefaultSearchDepth);

}  // namespace ui
