#pragma once

#include "bitboard.hpp"
#include "bitboards.hpp"
#include "move.hpp"
#include "side.hpp"

class node
{
    bitboard white;
    bitboard black;
    bitboard king_;
    bitboard rook_queen_;
    bitboard bishop_queen_;
    bitboard knight_;
    bitboard pawn_;
    bitboard castle;
    bitboard en_passant;

public:
    constexpr node(bitboard white, bitboard black, bitboard king, bitboard rook_queen, bitboard bishop_queen, bitboard knight, bitboard pawn, bitboard castle, bitboard en_passant) noexcept
        : white(white), black(black), king_(king), rook_queen_(rook_queen), bishop_queen_(bishop_queen), knight_(knight), pawn_(pawn), castle(castle), en_passant(en_passant) {}

    constexpr bitboard occupied() const noexcept {
        return white | black;
    }

    template <side_t side>
    constexpr bitboard occupied() const noexcept {
        return side == WHITE ? white : black;
    }

    template <side_t side>
    constexpr bitboard king() const noexcept {
        return king_ & occupied<side>();
    }

    template <side_t side>
    constexpr bitboard rook_queen() const noexcept {
        return rook_queen_ & occupied<side>();
    }

    template <side_t side>
    constexpr bitboard bishop_queen() const noexcept {
        return bishop_queen_ & occupied<side>();
    }

    template <side_t side>
    constexpr bitboard knight() const noexcept {
        return knight_ & occupied<side>();
    }

    template <side_t side>
    constexpr bitboard pawn() const noexcept {
        return pawn_ & occupied<side>();
    }

    template <side_t side>
    constexpr bitboard attackers() const noexcept;

    template <side_t side>
    bitboard attackers(square square) const noexcept;

    template <side_t side>
    bitboard checkers() const noexcept;

    template <side_t side>
    std::span<move> generate(std::span<move, 256> moves) const noexcept;

    template <side_t side>
    void execute(const move& move) noexcept;
};

template <side_t side>
constexpr bitboard node::attackers() const noexcept {
  bitboard empty = ~occupied() | king<~side>();
  bitboard out = 0ull;
  out |= bitboards::king(king<side>());
  out |= bitboards::knight(knight<side>());
  out |= bitboards::rook_queen(rook_queen<side>(), empty);
  out |= bitboards::bishop_queen(bishop_queen<side>(), empty);
  out |= bitboards::pawn<side>(pawn<side>());
  return out;
}

template <side_t side>
inline bitboard node::attackers(square square) const noexcept {
  bitboard out = 0ull;
  out |= bitboards::king(square) & king<side>();
  out |= bitboards::knight(square) & knight<side>();
  out |= bitboards::rook_queen(square, occupied()) & rook_queen<side>();
  out |= bitboards::bishop_queen(square, occupied()) & bishop_queen<side>();
  out |= bitboards::pawn<~side>(square) & pawn<side>();
  return out;
}

template <side_t side>
inline bitboard node::checkers() const noexcept {
  auto square = king<side>().find();
  return attackers<~side>(square);
}
