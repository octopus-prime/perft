#pragma once

#include <cstdint>
#include <ostream>

enum piece : std::uint8_t {
    EMPTY = ' ',
    PAWN = 'P',
    KNIGHT = 'N',
    BISHOP = 'B',
    ROOK = 'R',
    QUEEN = 'Q',
    KING = 'K'
};

std::ostream& operator<<(std::ostream& stream, piece piece);
