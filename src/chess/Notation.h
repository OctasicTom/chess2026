#pragma once
#include <optional>
#include <string>
#include <string_view>
#include "Types.h"

namespace chess {

// Square <-> algebraic name ("a1" = 0 ... "h8" = 63).
std::string squareToString(Square s);  // returns "??" for invalid squares
std::optional<Square> squareFromString(std::string_view name);  // lowercase a-h, 1-8 only

enum class MoveParseError {
    None,
    BadLength,
    BadSourceSquare,
    BadDestinationSquare,
    BadPromotion,
    SameSquare
};

struct MoveParseResult {
    MoveParseError error = MoveParseError::None;
    Move move;  // from/to/promotion only; flags are not set (no legality or board knowledge)

    bool ok() const { return error == MoveParseError::None; }
};

// Parses coordinate notation such as "e2e4" or "e7e8q". Surrounding whitespace is ignored
// and case is ignored. Purely syntactic: legality is not checked.
MoveParseResult parseMove(std::string_view text);

const char* describe(MoveParseError error);

}  // namespace chess
