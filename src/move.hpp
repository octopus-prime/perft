#pragma once

#include "square.hpp"
#include "piece.hpp"

class move {
    piece moved_;
    square from_;
    square to_;
    piece promoted_;

public:
    constexpr move(piece moved = EMPTY, square from = 0, square to = 0, piece promoted = EMPTY) noexcept 
    : moved_(moved), from_(from), to_(to), promoted_(promoted) {}

    constexpr piece moved() const noexcept { return moved_; }
    constexpr square from() const noexcept { return from_; }
    constexpr square to() const noexcept { return to_; }
    constexpr piece promoted() const noexcept { return promoted_; }
};

std::ostream& operator<<(std::ostream& stream, move move);
