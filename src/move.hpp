#pragma once

#include "square.hpp"

struct move {
    enum type_t : std::uint8_t {
        KING,
        CASTLE_SHORT,
        CASTLE_LONG,
        QUEEN,
        ROOK,
        BISHOP,
        KNIGHT,
        PAWN,
        PROMOTE_QUEEN,
        PROMOTE_ROOK,
        PROMOTE_BISHOP,
        PROMOTE_KNIGHT,
        DOUBLE_PUSH,
        EN_PASSANT
    };

    type_t type = KING;
    square from = 0;
    square to = 0;
};

std::ostream& operator<<(std::ostream& stream, const move& move);
