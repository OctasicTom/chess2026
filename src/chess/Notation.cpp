#include "Notation.h"
#include <cctype>

namespace chess {

namespace {

char lower(char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }

std::string_view trim(std::string_view s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.remove_prefix(1);
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.remove_suffix(1);
    return s;
}

}  // namespace

std::string squareToString(Square s) {
    if (s >= kNumSquares) return "??";
    return std::string{static_cast<char>('a' + fileOf(s)), static_cast<char>('1' + rankOf(s))};
}

std::optional<Square> squareFromString(std::string_view name) {
    if (name.size() != 2) return std::nullopt;
    const char f = name[0], r = name[1];
    if (f < 'a' || f > 'h' || r < '1' || r > '8') return std::nullopt;
    return makeSquare(f - 'a', r - '1');
}

MoveParseResult parseMove(std::string_view text) {
    MoveParseResult result;
    std::string s;
    for (char c : trim(text)) s.push_back(lower(c));

    if (s.size() != 4 && s.size() != 5) {
        result.error = MoveParseError::BadLength;
        return result;
    }
    const auto from = squareFromString(std::string_view(s).substr(0, 2));
    if (!from) { result.error = MoveParseError::BadSourceSquare; return result; }
    const auto to = squareFromString(std::string_view(s).substr(2, 2));
    if (!to) { result.error = MoveParseError::BadDestinationSquare; return result; }
    if (*from == *to) { result.error = MoveParseError::SameSquare; return result; }

    PieceType promo = PieceType::None;
    if (s.size() == 5) {
        switch (s[4]) {
            case 'q': promo = PieceType::Queen; break;
            case 'r': promo = PieceType::Rook; break;
            case 'b': promo = PieceType::Bishop; break;
            case 'n': promo = PieceType::Knight; break;
            default: result.error = MoveParseError::BadPromotion; return result;
        }
    }
    result.move.from = *from;
    result.move.to = *to;
    result.move.promotion = promo;
    if (promo != PieceType::None) result.move.flags = MoveFlagPromotion;
    return result;
}

const char* describe(MoveParseError error) {
    switch (error) {
        case MoveParseError::None: return "no error";
        case MoveParseError::BadLength: return "move must look like e2e4 or e7e8q";
        case MoveParseError::BadSourceSquare: return "invalid source square";
        case MoveParseError::BadDestinationSquare: return "invalid destination square";
        case MoveParseError::BadPromotion: return "promotion must be one of q, r, b, n";
        case MoveParseError::SameSquare: return "source and destination are the same";
    }
    return "unknown error";
}

}  // namespace chess
