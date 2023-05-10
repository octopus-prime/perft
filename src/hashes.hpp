#pragma once

#include "hash.hpp"
#include "side.hpp"
#include "square.hpp"
#include <array>

class hashes {
    using lookup_t = std::array<std::array<hash_t, 64>, 2>;

    static const lookup_t king_;
    static const lookup_t queen_;
    static const lookup_t rook_;
    static const lookup_t bishop_;
    static const lookup_t knight_;
    static const lookup_t pawn_;

public:
    template <side_t side>
    inline static hash_t king(square square) noexcept {
        return side == WHITE ? king_[WHITE][square] : king_[BLACK][square];
    }

    template <side_t side>
    inline static hash_t queen(square square) noexcept {
        return side == WHITE ? queen_[WHITE][square] : queen_[BLACK][square];
    }

    template <side_t side>
    inline static hash_t rook(square square) noexcept {
        return side == WHITE ? rook_[WHITE][square] : rook_[BLACK][square];
    }

    template <side_t side>
    inline static hash_t bishop(square square) noexcept {
        return side == WHITE ? bishop_[WHITE][square] : bishop_[BLACK][square];
    }

    template <side_t side>
    inline static hash_t knight(square square) noexcept {
        return side == WHITE ? knight_[WHITE][square] : knight_[BLACK][square];
    }

    template <side_t side>
    inline static hash_t pawn(square square) noexcept {
        return side == WHITE ? pawn_[WHITE][square] : pawn_[BLACK][square];
    }
};
