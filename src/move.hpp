#pragma once

#include "square.hpp"

class move {
public:
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

private:
    type_t type_;
    square from_;
    square to_;

public:
    constexpr move(type_t type = KING, square from = 0, square to = 0) noexcept
        : type_{type}, from_{from}, to_{to} {}
    constexpr type_t type() const noexcept { return type_; }
    constexpr square from() const noexcept { return from_; }
    constexpr square to() const noexcept { return to_; }
    constexpr bool operator==(const move& other) const noexcept = default;
};

std::ostream& operator<<(std::ostream& stream, const move& move);
