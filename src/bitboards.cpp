#include "bitboards.hpp"

constexpr bitboard bitboards::permutation(bitboard iteration, bitboard mask) noexcept {
  bitboard blockers = 0ull;
  while (iteration != 0ull) {
    if ((iteration & 1ull) != 0ull) {
      blockers |= bitboard{mask.find()};
    }
    iteration >>= 1;
    mask &= (mask - 1ull);
  }
  return blockers;
}

const bitboards::leaper_lookup_t bitboards::lookup_king = []() noexcept {
  bitboards::leaper_lookup_t v{};
  for (square i = 0; i < 64; ++i)
    v[i] = bitboards::king(bitboard{i});
  return v;
}();

const bitboards::leaper_lookup_t bitboards::lookup_knight = []() noexcept {
  bitboards::leaper_lookup_t v{};
  for (square i = 0; i < 64; ++i)
    v[i] = bitboards::knight(bitboard{i});
  return v;
}();

const bitboards::leaper_lookup_t bitboards::lookup_pawn_white = []() noexcept {
  bitboards::leaper_lookup_t v{};
  for (square i = 0; i < 64; ++i)
    v[i] = bitboards::pawn<WHITE>(bitboard{i});
  return v;
}();

const bitboards::leaper_lookup_t bitboards::lookup_pawn_black = []() noexcept {
  bitboards::leaper_lookup_t v{};
  for (square i = 0; i < 64; ++i)
    v[i] = bitboards::pawn<BLACK>(bitboard{i});
  return v;
}();

const bitboards::slider_lookup_t bitboards::lookup_rook_queen = []() noexcept {
  std::array<bitboards::slider_lookup_t::block_t, 64> blocks {};
  for (square i = 0; i < 64; ++i) {
    bitboard board{i};
    bitboard rooks = rook_queen(board, 0ull);
    auto size = rooks.count();
    blocks[i].data.resize(size);
    for (auto j = 0; j < size; ++j) {
      bitboard blockers = permutation(j, rooks);
      bitboard rooks2 = rook_queen(board, blockers);
      blocks[i].data[j] = rooks2;
    }
    blocks[i].mask = rooks;
  }
  return bitboards::slider_lookup_t{blocks};
}();

const bitboards::slider_lookup_t bitboards::lookup_bishop_queen = []() noexcept {
  std::array<bitboards::slider_lookup_t::block_t, 64> blocks {};
  for (square i = 0; i < 64; ++i) {
    bitboard board = 1ull << i;
    bitboard bishops = bishop_queen(board, 0ull);
    auto size = 1ull << bishops.count();
    blocks[i].data.resize(size);
    for (auto j = 0; j < size; ++j) {
      bitboard blockers = permutation(j, bishops);
      bitboard bishops2 = bishop_queen(board, blockers);
      blocks[i].data[j] = bishops2;
    }
    blocks[i].mask = bishops;
  }
  return bitboards::slider_lookup_t{blocks};
}();

const bitboards::line_lookup_t bitboards::lookup_line = []() noexcept {
  bitboards::line_lookup_t x{};
  for (square from = 0; from < 64; ++from) {
    for (square to = 0; to < 64; ++to) {
      x[from][to] = bitboard{from} | bitboard{to};
      if (from == to)
        continue;
      bitboard y = bishop_queen(from, 0ull) | rook_queen(from, 0ull);
      if (!y[to])
        continue;
      for (square sq = from; sq != to; ) {
        x[from][to] |= bitboard{square{sq & 63}};
        sq += (from.file() == to.file()) ? 0 : (from.file() > to.file()) ? -1 : +1;
        sq += (from.rank() == to.rank()) ? 0 : (from.rank() > to.rank()) ? -8 : +8;
      }
    }
  }
  return x;
}();

static_assert(bitboards::king("e4"_b) == "d3e3f3d4f4d5e5f5"_b);
static_assert(bitboards::knight("e4"_b) == "d2f2c3g3c5g5d6f6"_b);
static_assert(bitboards::rook_queen("e4"_b, ""_b) == "e1e2e3a4b4c4d4f4g4h4e5e6e7e8"_b);
static_assert(bitboards::bishop_queen("e4"_b, ""_b) == "b1h1c2g2d3f3d5f5c6g6b7h7a8"_b);
static_assert(bitboards::pawn<WHITE>("e4"_b) == "d5f5"_b);
static_assert(bitboards::pawn<BLACK>("e4"_b) == "d3f3"_b);
