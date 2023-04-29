#pragma once

#include "bitboard.hpp"
#include "move.hpp"
#include "side.hpp"

class node
{
    bitboard white;
    bitboard black;
    bitboard king;
    bitboard rook_queen;
    bitboard bishop_queen;
    bitboard knight;
    bitboard pawn;
    bitboard castle;
    bitboard en_passant;

public:
    constexpr node(bitboard white, bitboard black, bitboard king, bitboard rook_queen, bitboard bishop_queen, bitboard knight, bitboard pawn, bitboard castle, bitboard en_passant) noexcept
        : white(white), black(black), king(king), rook_queen(rook_queen), bishop_queen(bishop_queen), knight(knight), pawn(pawn), castle(castle), en_passant(en_passant) {}

    template <side_t side>
    constexpr bitboard occupied() const noexcept {
        return side == WHITE ? white : black;
    }

    constexpr bitboard occupied() const noexcept {
        return white | black;
    }

    template <side_t side>
    std::span<move> generate(std::span<move, 256> moves) const noexcept;

    template <side_t side>
    void execute(const move& move) noexcept;
};
