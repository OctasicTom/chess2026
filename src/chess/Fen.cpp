#include "Fen.h"
#include <charconv>
#include <cctype>
#include <vector>
#include "Notation.h"

namespace chess {

namespace {

std::optional<Piece> pieceFromChar(char c) {
    const Color color = std::isupper(static_cast<unsigned char>(c)) ? Color::White : Color::Black;
    switch (std::tolower(static_cast<unsigned char>(c))) {
        case 'p': return Piece{PieceType::Pawn, color};
        case 'n': return Piece{PieceType::Knight, color};
        case 'b': return Piece{PieceType::Bishop, color};
        case 'r': return Piece{PieceType::Rook, color};
        case 'q': return Piece{PieceType::Queen, color};
        case 'k': return Piece{PieceType::King, color};
        default: return std::nullopt;
    }
}

std::vector<std::string_view> splitFields(std::string_view s) {
    std::vector<std::string_view> out;
    std::size_t i = 0;
    while (i < s.size()) {
        while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
        const std::size_t start = i;
        while (i < s.size() && !std::isspace(static_cast<unsigned char>(s[i]))) ++i;
        if (i > start) out.push_back(s.substr(start, i - start));
    }
    return out;
}

bool parseInt(std::string_view s, int& value) {
    if (s.empty()) return false;
    const auto res = std::from_chars(s.data(), s.data() + s.size(), value);
    return res.ec == std::errc() && res.ptr == s.data() + s.size() && value >= 0;
}

}  // namespace

std::optional<Position> parseFen(std::string_view fen) {
    const std::vector<std::string_view> fields = splitFields(fen);
    if (fields.size() != 4 && fields.size() != 6) return std::nullopt;

    Position pos;

    int file = 0, rank = 7;
    for (char c : fields[0]) {
        if (c == '/') {
            if (file != 8 || rank == 0) return std::nullopt;
            --rank;
            file = 0;
        } else if (c >= '1' && c <= '8') {
            file += c - '0';
            if (file > 8) return std::nullopt;
        } else {
            const auto piece = pieceFromChar(c);
            if (!piece || file >= 8) return std::nullopt;
            pos.board.set(makeSquare(file++, rank), *piece);
        }
    }
    if (rank != 0 || file != 8) return std::nullopt;

    if (fields[1] == "w") pos.sideToMove = Color::White;
    else if (fields[1] == "b") pos.sideToMove = Color::Black;
    else return std::nullopt;

    if (fields[2] != "-") {
        for (char c : fields[2]) {
            switch (c) {
                case 'K': pos.castlingRights |= CastleWhiteKing; break;
                case 'Q': pos.castlingRights |= CastleWhiteQueen; break;
                case 'k': pos.castlingRights |= CastleBlackKing; break;
                case 'q': pos.castlingRights |= CastleBlackQueen; break;
                default: return std::nullopt;
            }
        }
    }

    if (fields[3] != "-") {
        const auto sq = squareFromString(fields[3]);
        if (!sq) return std::nullopt;
        const int r = rankOf(*sq);
        if (r != (pos.sideToMove == Color::White ? 5 : 2)) return std::nullopt;
        pos.enPassantSquare = *sq;
    }

    if (fields.size() == 6) {
        if (!parseInt(fields[4], pos.halfmoveClock)) return std::nullopt;
        if (!parseInt(fields[5], pos.fullmoveNumber) || pos.fullmoveNumber < 1) return std::nullopt;
    }
    return pos;
}

std::string toFen(const Position& pos) {
    std::string s;
    for (int rank = 7; rank >= 0; --rank) {
        int empties = 0;
        for (int file = 0; file < 8; ++file) {
            const Piece p = pos.board.at(makeSquare(file, rank));
            if (p.empty()) { ++empties; continue; }
            if (empties) { s += static_cast<char>('0' + empties); empties = 0; }
            s += p.toChar();
        }
        if (empties) s += static_cast<char>('0' + empties);
        if (rank > 0) s += '/';
    }
    s += pos.sideToMove == Color::White ? " w " : " b ";
    if (!pos.castlingRights) s += '-';
    if (pos.castlingRights & CastleWhiteKing) s += 'K';
    if (pos.castlingRights & CastleWhiteQueen) s += 'Q';
    if (pos.castlingRights & CastleBlackKing) s += 'k';
    if (pos.castlingRights & CastleBlackQueen) s += 'q';
    s += ' ';
    s += pos.enPassantSquare == kNoSquare ? std::string("-") : squareToString(pos.enPassantSquare);
    s += ' ' + std::to_string(pos.halfmoveClock) + ' ' + std::to_string(pos.fullmoveNumber);
    return s;
}

}  // namespace chess
