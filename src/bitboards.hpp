#pragma once

#include "bitboard.hpp"
#include "side.hpp"
#include <array>
#include <vector>
#include <immintrin.h>

class bitboards
{
  using dualboard = __v2du;
  using quadboard = __v4du;
  using leaper_lookup_t = std::array<bitboard, 64>;
  using line_lookup_t = std::array<leaper_lookup_t, 64>;
  struct slider_lookup_t;

  static const line_lookup_t lookup_line;
  static const leaper_lookup_t lookup_king;
  static const leaper_lookup_t lookup_knight;
  static const leaper_lookup_t lookup_pawn_white;
  static const leaper_lookup_t lookup_pawn_black;
  static const slider_lookup_t lookup_rook_queen;
  static const slider_lookup_t lookup_bishop_queen;

  static constexpr bitboard permutation(bitboard iteration, bitboard mask) noexcept;
  static constexpr auto expand(auto in, auto empty) noexcept;

public:
  static constexpr bitboard ALL = ~0ull;

  static bitboard line(square from, square to) noexcept;

  static bitboard king(square square) noexcept;
  static constexpr bitboard king(bitboard squares) noexcept;

  static bitboard knight(square square) noexcept;
  static constexpr bitboard knight(bitboard squares) noexcept;

  static bitboard rook_queen(square square, bitboard occupied) noexcept;
  static constexpr bitboard rook_queen(bitboard squares, bitboard occupied) noexcept;

  static bitboard bishop_queen(square square, bitboard occupied) noexcept;
  static constexpr bitboard bishop_queen(bitboard squares, bitboard occupied) noexcept;

  static constexpr bitboard slider(bitboard rook_queen, bitboard bishop_queen, bitboard occupied) noexcept;

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

constexpr bitboard bitboards::king(bitboard in) noexcept
{
  constexpr quadboard s = {1, 8, 7, 9};
  constexpr quadboard l = {~"a"_f, ~""_f, ~"h"_f, ~"a"_f};
  constexpr quadboard r = {~"h"_f, ~""_f, ~"a"_f, ~"h"_f};
  quadboard b = {in, in, in, in};
  quadboard t = ((b << s) & l) | ((b >> s) & r);
  return t[0] | t[1] | t[2] | t[3];
}

constexpr bitboard bitboards::knight(bitboard in) noexcept
{
  constexpr quadboard s = {10, 17, 15, 6};
  constexpr quadboard l = {~"ab"_f, ~"a"_f, ~"h"_f, ~"gh"_f};
  constexpr quadboard r = {~"gh"_f, ~"h"_f, ~"a"_f, ~"ab"_f};
  quadboard b = {in, in, in, in};
  quadboard t = ((b << s) & l) | ((b >> s) & r);
  return t[0] | t[1] | t[2] | t[3];
}

constexpr auto bitboards::expand(auto in, auto empty) noexcept {
	constexpr quadboard shift = { 1, 8, 7, 9 };
	constexpr quadboard shift2 = { 2, 16, 14, 18 };
	constexpr quadboard shift4 = { 4, 32, 28, 36 };
	constexpr quadboard not_left = { ~"a"_f, ~""_f, ~"h"_f, ~"a"_f };
	constexpr quadboard not_right = { ~"h"_f, ~""_f, ~"a"_f, ~"h"_f };
	quadboard left(in);
	quadboard right(in);
	quadboard board(empty & not_left);

	left |= board & (left << shift);
	board &= (board << shift);
	left |= board & (left << shift2);
	board &= (board << shift2);
	left |= board & (left << shift4);
	left = (left << shift) & not_left;

	board = empty & not_right;

	right |= board & (right >> shift);
	board &= (board >> shift);
	right |= board & (right >> shift2);
	board &= (board >> shift2);
	right |= board & (right >> shift4);
	right = (right >> shift) & not_right;

	return left | right;
}

constexpr bitboard bitboards::rook_queen(bitboard in, bitboard occupied) noexcept
{
  quadboard b = {in, in, 0, 0};
  quadboard o = {occupied, occupied, 0, 0};
  quadboard t = expand(b, ~o);
  return t[0] | t[1];
}

constexpr bitboard bitboards::bishop_queen(bitboard in, bitboard occupied) noexcept
{
  quadboard b = {0, 0, in, in};
  quadboard o = {0, 0, occupied, occupied};
  quadboard t = expand(b, ~o);
  return t[2] | t[3];
}

constexpr bitboard bitboards::slider(bitboard rook_queen, bitboard bishop_queen, bitboard occupied) noexcept
{
  quadboard b = {rook_queen, rook_queen, bishop_queen, bishop_queen};
  quadboard o = {occupied, occupied, occupied, occupied};
  quadboard t = expand(b, ~o);
  return t[0] | t[1] | t[2] | t[3];
}

template <>
constexpr bitboard bitboards::pawn<WHITE>(bitboard in) noexcept
{
  constexpr dualboard s = {7, 9};
  constexpr dualboard m = {~"h"_f, ~"a"_f};
  dualboard b = {in, in};
  dualboard t = (b << s) & m;
  return t[0] | t[1];
}

template <>
constexpr bitboard bitboards::pawn<BLACK>(bitboard in) noexcept
{
  constexpr dualboard s = {7, 9};
  constexpr dualboard m = {~"a"_f, ~"h"_f};
  dualboard b = {in, in};
  dualboard t = (b >> s) & m;
  return t[0] | t[1];
}
