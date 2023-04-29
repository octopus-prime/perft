#pragma once

#include "square.hpp"

class move {
    square from_;
    square to_;

public:
    constexpr move(square from = 0, square to = 0) noexcept : from_(from), to_(to) {}

    constexpr square from() const noexcept { return from_; }
    constexpr square to() const noexcept { return to_; }
};
