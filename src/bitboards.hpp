#pragma once

#include "bitboard.hpp"
#include "side.hpp"
#include <array>
#include <vector>
#include <immintrin.h> // _pext

class bitboards
{
  using line_lookup_t = std::array<std::array<bitboard, 64>, 64>;
  using leaper_lookup_t = std::array<bitboard, 64>;
  struct slider_lookup_t;

  static const line_lookup_t lookup_line;
  static const leaper_lookup_t lookup_king;
  static const leaper_lookup_t lookup_knight;
  static const leaper_lookup_t lookup_pawn_white;
  static const leaper_lookup_t lookup_pawn_black;
  static const slider_lookup_t lookup_rook_queen;
  static const slider_lookup_t lookup_bishop_queen;

  static constexpr bitboard expand(bitboard in, auto &&view) noexcept;
  static constexpr bitboard expand(bitboard in, bitboard empty, auto &&view) noexcept;
  static constexpr bitboard permutation(bitboard iteration, bitboard mask) noexcept;

public:
  static bitboard line(square from, square to) noexcept;

  static bitboard king(square square) noexcept;
  static constexpr bitboard king(bitboard squares) noexcept;

  static bitboard knight(square square) noexcept;
  static constexpr bitboard knight(bitboard squares) noexcept;

  static bitboard rook_queen(square square, bitboard occupied) noexcept;
  static constexpr bitboard rook_queen(bitboard squares, bitboard occupied) noexcept;

  static bitboard bishop_queen(square square, bitboard occupied) noexcept;
  static constexpr bitboard bishop_queen(bitboard squares, bitboard occupied) noexcept;

  template <side_t side>
  static bitboard pawn(square square) noexcept;
  template <side_t side>
  static constexpr bitboard pawn(bitboard squares) noexcept;
};

struct bitboards::slider_lookup_t
{
  struct block_t
  {
    bitboard mask;
    std::vector<bitboard> data;
  };

  std::array<block_t, 64> blocks;

  bitboard
  operator()(square square, bitboard occupied) const noexcept
  {
    const auto &block = blocks[square];
    const auto index = _pext_u64(occupied, block.mask);
    return block.data[index];
  }
};

inline bitboard bitboards::line(square from, square to) noexcept
{
  return lookup_line[from][to];
}

inline bitboard bitboards::king(square square) noexcept
{
  return lookup_king[square];
}

inline bitboard bitboards::knight(square square) noexcept
{
  return lookup_knight[square];
}

inline bitboard bitboards::rook_queen(square square, bitboard occupied) noexcept
{
  return lookup_rook_queen(square, occupied);
}

inline bitboard bitboards::bishop_queen(square square, bitboard occupied) noexcept
{
  return lookup_bishop_queen(square, occupied);
}

template <>
inline bitboard bitboards::pawn<WHITE>(square square) noexcept {
  return lookup_pawn_white[square];
}

template <>
inline bitboard bitboards::pawn<BLACK>(square square) noexcept {
  return lookup_pawn_black[square];
}

constexpr bitboard bitboards::expand(bitboard in, auto &&view) noexcept
{
  return std::transform_reduce(view.begin(), view.end(), 0ull, std::bit_or{}, [in](auto &&tuple) noexcept
                               {
    const bitboard left = (in << std::get<0>(tuple)) & std::get<1>(tuple);
    const bitboard right = (in >> std::get<0>(tuple)) & std::get<2>(tuple);
    return left | right; });
}

constexpr bitboard bitboards::king(bitboard in) noexcept
{
  constexpr int shifts[] = {1, 8, 7, 9};
  constexpr bitboard masks_left[] = {~"a"_f, ~""_f, ~"h"_f, ~"a"_f};
  constexpr bitboard masks_right[] = {~"h"_f, ~""_f, ~"a"_f, ~"h"_f};
  return bitboards::expand(in, std::views::zip(shifts, masks_left, masks_right));
}

constexpr bitboard bitboards::knight(bitboard in) noexcept
{
  constexpr int shifts[] = {10, 17, 15, 6};
  constexpr bitboard masks_left[] = {~"ab"_f, ~"a"_f, ~"h"_f, ~"gh"_f};
  constexpr bitboard masks_right[] = {~"gh"_f, ~"h"_f, ~"a"_f, ~"ab"_f};
  return expand(in, std::views::zip(shifts, masks_left, masks_right));
}

constexpr bitboard bitboards::expand(bitboard in, bitboard empty, auto &&view) noexcept
{
  return std::transform_reduce(view.begin(), view.end(), 0ull, std::bit_or{}, [in, empty](auto &&tuple) noexcept
                               {
    bitboard left(in);
    bitboard right(in);
    bitboard board(empty & std::get<1>(tuple));

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

    return left | right; });
}

constexpr bitboard bitboards::rook_queen(bitboard in, bitboard occupied) noexcept
{
  constexpr int shifts[] = {1, 8};
  constexpr bitboard masks_left[] = {~"a"_f, ~""_f};
  constexpr bitboard masks_right[] = {~"h"_f, ~""_f};
  return expand(in, ~occupied, std::views::zip(shifts, masks_left, masks_right));
}

constexpr bitboard bitboards::bishop_queen(bitboard in, bitboard occupied) noexcept
{
  constexpr int shifts[] = {7, 9};
  constexpr bitboard masks_left[] = {~"h"_f, ~"a"_f};
  constexpr bitboard masks_right[] = {~"a"_f, ~"h"_f};
  return expand(in, ~occupied, std::views::zip(shifts, masks_left, masks_right));
}

template <>
constexpr bitboard bitboards::pawn<WHITE>(bitboard in) noexcept
{
  constexpr int shifts[] = {7, 9};
  constexpr uint64_t masks[] = {~"h"_f, ~"a"_f};
  const auto view = std::views::zip(shifts, masks);
  return std::transform_reduce(view.begin(), view.end(), 0ull, std::bit_or{}, [in](auto &&tuple) noexcept
                               { return (in << std::get<0>(tuple)) & std::get<1>(tuple); });
}

template <>
constexpr bitboard bitboards::pawn<BLACK>(bitboard in) noexcept
{
  constexpr int shifts[] = {7, 9};
  constexpr uint64_t masks[] = {~"a"_f, ~"h"_f};
  const auto view = std::views::zip(shifts, masks);
  return std::transform_reduce(view.begin(), view.end(), 0ull, std::bit_or{}, [in](auto &&tuple) noexcept
                               { return (in >> std::get<0>(tuple)) & std::get<1>(tuple); });
}
