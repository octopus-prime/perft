#pragma once

#include "bitboard.hpp"
#include "side.hpp"
#include <array>
#include <vector>
#include <immintrin.h> // _pext

class bitboards
{
    using line_lookup_t = std::array<std::array<bitboard_t, 64>, 64>;
    using leaper_lookup_t = std::array<bitboard_t, 64>;
    struct slider_lookup_t;

    static const line_lookup_t lookup_line;
    static const leaper_lookup_t lookup_king;
    static const leaper_lookup_t lookup_knight;
    static const slider_lookup_t lookup_rook_queen;
    static const slider_lookup_t lookup_bishop_queen;

    static constexpr bitboard_t expand(bitboard_t in, auto&& view) noexcept;
    static constexpr bitboard_t expand(bitboard_t in, bitboard_t empty, auto&& view) noexcept;
    static constexpr bitboard_t permutation(bitboard_t iteration, bitboard_t mask) noexcept;

public:
    static bitboard_t line(square_t from, square_t to) noexcept;

    static bitboard_t king(square_t square) noexcept;
    static constexpr bitboard_t king(bitboard_t squares) noexcept;

    static bitboard_t knight(square_t square) noexcept;
    static constexpr bitboard_t knight(bitboard_t squares) noexcept;

    static bitboard_t rook_queen(square_t square, bitboard_t occupied) noexcept;
    static constexpr bitboard_t rook_queen(bitboard_t squares, bitboard_t occupied) noexcept;

    static bitboard_t bishop_queen(square_t square, bitboard_t occupied) noexcept;
    static constexpr bitboard_t bishop_queen(bitboard_t squares, bitboard_t occupied) noexcept;

    // template <side_t side>
    // static constexpr bitboard_t pawn(square_t square) noexcept;
    // template <side_t side>
    // static constexpr bitboard_t pawn(bitboard_t squares) noexcept;
};

struct bitboards::slider_lookup_t
{
    struct block_t
    {
        bitboard_t mask;
        std::vector<bitboard_t> data;
    };

    std::array<block_t, 64> blocks;

    bitboard_t
    operator()(square_t square, bitboard_t occupied) const noexcept
    {
        const auto &block = blocks[square];
        const auto index = _pext_u64(occupied, block.mask);
        return block.data[index];
    }
};

inline bitboard_t bitboards::line(square_t from, square_t to) noexcept {
    return lookup_line[from][to];
}

inline bitboard_t bitboards::king(square_t square) noexcept {
    return lookup_king[square];
}

inline bitboard_t bitboards::knight(square_t square) noexcept {
    return lookup_knight[square];
}

inline bitboard_t bitboards::rook_queen(square_t square, bitboard_t occupied) noexcept {
    return lookup_rook_queen(square, occupied);
}

inline bitboard_t bitboards::bishop_queen(square_t square, bitboard_t occupied) noexcept {
    return lookup_bishop_queen(square, occupied);
}

constexpr bitboard_t bitboards::expand(bitboard_t in, auto&& view) noexcept {
  return std::transform_reduce(view.begin(), view.end(), 0ull, std::bit_or{}, [in](auto&& tuple) noexcept {
    const bitboard_t left = (in << std::get<0>(tuple)) & std::get<1>(tuple);
    const bitboard_t right = (in >> std::get<0>(tuple)) & std::get<2>(tuple);
    return left | right;
  });
}

constexpr bitboard_t bitboards::king(bitboard_t in) noexcept {
  constexpr int shifts[] = {1, 8, 7, 9};
  constexpr bitboard_t masks_left[] = {~"a"_f, ~""_f, ~"h"_f, ~"a"_f};
  constexpr bitboard_t masks_right[] = {~"h"_f, ~""_f, ~"a"_f, ~"h"_f};
  return bitboards::expand(in, std::views::zip(shifts, masks_left, masks_right));
}

constexpr bitboard_t bitboards::knight(bitboard_t in) noexcept {
  constexpr int shifts[] = {10, 17, 15, 6};
  constexpr bitboard_t masks_left[] = {~"ab"_f, ~"a"_f, ~"h"_f, ~"gh"_f};
  constexpr bitboard_t masks_right[] = {~"gh"_f, ~"h"_f, ~"a"_f, ~"ab"_f};
  return expand(in, std::views::zip(shifts, masks_left, masks_right));
}

constexpr bitboard_t bitboards::expand(bitboard_t in, bitboard_t empty, auto&& view) noexcept {
  return std::transform_reduce(view.begin(), view.end(), 0ull, std::bit_or{}, [in, empty](auto&& tuple) noexcept {
    bitboard_t left(in);
    bitboard_t right(in);
    bitboard_t board(empty & std::get<1>(tuple));

    left |= board & (left << std::get<0>(tuple));
    board &= (board << std::get<0>(tuple));
    left |= board & (left << (std::get<0>(tuple) * 2));
    board &= (board << (std::get<0>(tuple) * 2));
    left |= board & (left << (std::get<0>(tuple) * 4));
    left = (left << std::get<0>(tuple)) & std::get<1>(tuple);

    board = empty & std::get<2>(tuple);

    right |= board & (right >> std::get<0>(tuple));
    board &= (board >> std::get<0>(tuple));
    right |= board & (right >> (std::get<0>(tuple) * 2));
    board &= (board >> (std::get<0>(tuple) * 2));
    right |= board & (right >> (std::get<0>(tuple) * 4));
    right = (right >> std::get<0>(tuple)) & std::get<2>(tuple);

    return left | right;
  });
}

constexpr bitboard_t bitboards::rook_queen(bitboard_t in, bitboard_t occupied) noexcept {
  constexpr int shifts[] = {1, 8};
  constexpr bitboard_t masks_left[] = {~"a"_f, ~""_f};
  constexpr bitboard_t masks_right[] = {~"h"_f, ~""_f};
  return expand(in, ~occupied, std::views::zip(shifts, masks_left, masks_right));
}

constexpr bitboard_t bitboards::bishop_queen(bitboard_t in, bitboard_t occupied) noexcept {
  constexpr int shifts[] = {7, 9};
  constexpr bitboard_t masks_left[] = {~"h"_f, ~"a"_f};
  constexpr bitboard_t masks_right[] = {~"a"_f, ~"h"_f};
  return expand(in, ~occupied, std::views::zip(shifts, masks_left, masks_right));
}
