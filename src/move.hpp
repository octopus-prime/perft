#pragma once

#include "square.hpp"
// #include "piece.hpp"

// class move {
//     piece moved_;
//     square from_;
//     square to_;
//     piece promoted_;

// public:
//     constexpr move(piece moved = EMPTY, square from = 0, square to = 0, piece promoted = EMPTY) noexcept 
//     : moved_(moved), from_(from), to_(to), promoted_(promoted) {}

//     constexpr piece moved() const noexcept { return moved_; }
//     constexpr square from() const noexcept { return from_; }
//     constexpr square to() const noexcept { return to_; }
//     constexpr piece promoted() const noexcept { return promoted_; }
// };

    enum piece : std::uint8_t {
        PAWN = 'P',
        KNIGHT = 'N',
        BISHOP = 'B',
        ROOK = 'R',
        QUEEN = 'Q',
        KING = 'K'
    };

    enum special : std::uint8_t {
        NONE = 0,

        // king
        CASTLE_SHORT = 1,
        CASTLE_LONG = 2,

        // pawn
        PROMOTE_QUEEN = 1,
        PROMOTE_ROOK = 2,
        PROMOTE_BISHOP = 3,
        PROMOTE_KNIGHT = 4,
        DOUBLE_PUSH = 5,
        EN_PASSANT = 6
    };

class move {

    piece moved_;
    square from_;
    square to_;
    special flag_;

public:
    constexpr move(piece moved = PAWN, square from = 0, square to = 0, special flag = NONE) noexcept 
    : moved_(moved), from_(from), to_(to), flag_(flag) {}

    constexpr piece moved() const noexcept { return moved_; }
    constexpr square from() const noexcept { return from_; }
    constexpr square to() const noexcept { return to_; }
    constexpr special flag() const noexcept { return flag_; }
};

std::ostream& operator<<(std::ostream& stream, move move);
