#include "bitboards.hpp"

// constexpr uint64_t expand_pawns(uint64_t in, white_side) noexcept {
//   constexpr int shifts[] = {7, 9};
//   constexpr uint64_t masks[] = {~"h"_f, ~"a"_f};
//   const auto view = std::views::zip(shifts, masks);
//   return std::transform_reduce(view.begin(), view.end(), 0ull, std::bit_or{}, [in](auto&& tuple) noexcept {
//     return (in << std::get<0>(tuple)) & std::get<1>(tuple);
//   });
// }

// constexpr uint64_t expand_pawns(uint64_t in, black_side) noexcept {
//   constexpr int shifts[] = {7, 9};
//   constexpr uint64_t masks[] = {~"a"_f, ~"h"_f};
//   const auto view = std::views::zip(shifts, masks);
//   return std::transform_reduce(view.begin(), view.end(), 0ull, std::bit_or{}, [in](auto&& tuple) noexcept {
//     return (in >> std::get<0>(tuple)) & std::get<1>(tuple);
//   });
// }

constexpr uint64_t expand(uint64_t in, uint64_t empty, auto&& view) noexcept {
  return std::transform_reduce(view.begin(), view.end(), 0ull, std::bit_or{}, [in, empty](auto&& tuple) noexcept {
    uint64_t left(in);
    uint64_t right(in);
    uint64_t board(empty & std::get<1>(tuple));

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

constexpr uint64_t expand_rooks(uint64_t in, uint64_t empty) noexcept {
  constexpr int shifts[] = {1, 8};
  constexpr uint64_t masks_left[] = {~"a"_f, ~""_f};
  constexpr uint64_t masks_right[] = {~"h"_f, ~""_f};
  return expand(in, empty, std::views::zip(shifts, masks_left, masks_right));
}

constexpr uint64_t expand_bishops(uint64_t in, uint64_t empty) noexcept {
  constexpr int shifts[] = {7, 9};
  constexpr uint64_t masks_left[] = {~"h"_f, ~"a"_f};
  constexpr uint64_t masks_right[] = {~"a"_f, ~"h"_f};
  return expand(in, empty, std::views::zip(shifts, masks_left, masks_right));
}

constexpr bitboard_t bitboards::permutation(bitboard_t iteration, bitboard_t mask) noexcept {
  bitboard_t blockers = 0ull;
  while (iteration != 0ull) {
    if ((iteration & 1ull) != 0ull) {
      blockers |= bitboard_t{mask.find()};
    }
    iteration >>= 1;
    mask &= (mask - 1ull);
  }
  return blockers;
}

// const std::array<uint64_t, 64> lookup_pawns_w = []() noexcept {
//   std::array<uint64_t, 64> v;
//   for (int i = 0; i < 64; ++i)
//     v[i] = expand_pawns(1ull << i, white_side{});
//   return v;
// }();

// const std::array<uint64_t, 64> lookup_pawns_b = []() noexcept {
//   std::array<uint64_t, 64> v;
//   for (int i = 0; i < 64; ++i)
//     v[i] = expand_pawns(1ull << i, black_side{});
//   return v;
// }();

const bitboards::leaper_lookup_t bitboards::lookup_king = []() noexcept {
  bitboards::leaper_lookup_t v{};
  for (square_t i = 0; i < 64; ++i)
    v[i] = bitboards::king(bitboard_t{i});
  return v;
}();

const bitboards::leaper_lookup_t bitboards::lookup_knight = []() noexcept {
  bitboards::leaper_lookup_t v{};
  for (square_t i = 0; i < 64; ++i)
    v[i] = bitboards::knight(bitboard_t{i});
  return v;
}();

const bitboards::slider_lookup_t bitboards::lookup_rook_queen = []() noexcept {
  std::array<bitboards::slider_lookup_t::block_t, 64> blocks {};
  for (square_t i = 0; i < 64; ++i) {
    bitboard_t board{i};
    bitboard_t rooks = expand_rooks(board, ~0ull);
    auto size = rooks.count();
    blocks[i].data.resize(size);
    for (auto j = 0; j < size; ++j) {
      bitboard_t blockers = permutation(j, rooks);
      bitboard_t rooks2 = expand_rooks(board, ~blockers);
      blocks[i].data[j] = rooks2;
    }
    blocks[i].mask = rooks;
  }
  return bitboards::slider_lookup_t{blocks};
}();

const bitboards::slider_lookup_t bitboards::lookup_bishop_queen = []() noexcept {
  std::array<bitboards::slider_lookup_t::block_t, 64> blocks {};
  for (square_t i = 0; i < 64; ++i) {
    bitboard_t board = 1ull << i;
    bitboard_t bishops = expand_bishops(board, ~0ull);
    auto size = 1ull << bishops.count();
    blocks[i].data.resize(size);
    for (auto j = 0; j < size; ++j) {
      bitboard_t blockers = permutation(j, bishops);
      bitboard_t bishops2 = expand_bishops(board, ~blockers);
      blocks[i].data[j] = bishops2;
    }
    blocks[i].mask = bishops;
  }
  return bitboards::slider_lookup_t{blocks};
}();

const bitboards::line_lookup_t bitboards::lookup_line = []() noexcept {
  bitboards::line_lookup_t x{};
  for (square_t from = 0; from < 64; ++from) {
    for (square_t to = 0; to < 64; ++to) {
      x[from][to] = bitboard_t{from} | bitboard_t{to};
      if (from == to)
        continue;
      bitboard_t y = bitboards::bishop_queen(from, 0ull) | bitboards::rook_queen(from, 0ull);
      if (!y[to])
        continue;
      for (square_t square = from; square != to; ) {
        x[from][to] |= bitboard_t{square_t{square & 63}};
        square += (from.file() == to.file()) ? 0 : (from.file() > to.file()) ? -1 : +1;
        square += (from.rank() == to.rank()) ? 0 : (from.rank() > to.rank()) ? -8 : +8;
      }
    }
  }
  return x;
}();

static_assert(bitboards::king("e4"_b) == "d3e3f3d4f4d5e5f5"_b);
static_assert(bitboards::knight("e4"_b) == "d2f2c3g3c5g5d6f6"_b);
static_assert(bitboards::rook_queen("e4"_b, ""_b) == "e1e2e3a4b4c4d4f4g4h4e5e6e7e8"_b);
static_assert(bitboards::bishop_queen("e4"_b, ""_b) == "b1h1c2g2d3f3d5f5c6g6b7h7a8"_b);
