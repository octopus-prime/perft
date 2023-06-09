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
  leaper_lookup_t lookup{};
  for (square sq : ALL)
    lookup[sq] = king(bitboard{sq});
  return lookup;
}();

const bitboards::leaper_lookup_t bitboards::lookup_knight = []() noexcept {
  leaper_lookup_t lookup{};
  for (square sq : ALL)
    lookup[sq] = knight(bitboard{sq});
  return lookup;
}();

const bitboards::leaper_lookup_t bitboards::lookup_pawn_white = []() noexcept {
  leaper_lookup_t lookup{};
  for (square sq : ALL)
    lookup[sq] = pawn<WHITE>(bitboard{sq});
  return lookup;
}();

const bitboards::leaper_lookup_t bitboards::lookup_pawn_black = []() noexcept {
  leaper_lookup_t lookup{};
  for (square sq : ALL)
    lookup[sq] = pawn<BLACK>(bitboard{sq});
  return lookup;
}();

const bitboards::slider_lookup_t bitboards::lookup_rook_queen = []() noexcept {
  std::array<slider_lookup_t::block_t, 64> blocks {};
  for (square sq : ALL) {
    bitboard board{sq};
    bitboard rooks = rook_queen(board, 0ull);
    auto size = 1ull << rooks.count();
    blocks[sq].data.resize(size);
    for (std::uint64_t index = 0; index < size; ++index) {
      bitboard blockers = permutation(index, rooks);
      blocks[sq].data[index] = rook_queen(board, blockers);
    }
    blocks[sq].mask = rooks;
  }
  return slider_lookup_t{blocks};
}();

const bitboards::slider_lookup_t bitboards::lookup_bishop_queen = []() noexcept {
  std::array<slider_lookup_t::block_t, 64> blocks {};
  for (square sq : ALL) {
    bitboard board{sq};
    bitboard bishops = bishop_queen(board, 0ull);
    auto size = 1ull << bishops.count();
    blocks[sq].data.resize(size);
    for (std::uint64_t index = 0; index < size; ++index) {
      bitboard blockers = permutation(index, bishops);
      blocks[sq].data[index] = bishop_queen(board, blockers);
    }
    blocks[sq].mask = bishops;
  }
  return slider_lookup_t{blocks};
}();

const bitboards::line_lookup_t bitboards::lookup_line = []() noexcept {
  line_lookup_t lookup{};
  for (square from : ALL) {
    for (square to : ALL) {
      lookup[from][to] = bitboard{from} | bitboard{to};
      if (from == to)
        continue;
      bitboard board = bishop_queen(from, 0ull) | rook_queen(from, 0ull);
      if (!board[to])
        continue;
      for (square sq = from; sq != to; ) {
        lookup[from][to] |= bitboard{square{sq & 63}};
        sq += (from.file() == to.file()) ? 0 : (from.file() > to.file()) ? -1 : +1;
        sq += (from.rank() == to.rank()) ? 0 : (from.rank() > to.rank()) ? -8 : +8;
      }
    }
  }
  return lookup;
}();

static_assert(bitboards::king("e4"_b) == "d3e3f3d4f4d5e5f5"_b);
static_assert(bitboards::knight("e4"_b) == "d2f2c3g3c5g5d6f6"_b);
static_assert(bitboards::rook_queen("e4"_b, ""_b) == "e1e2e3a4b4c4d4f4g4h4e5e6e7e8"_b);
static_assert(bitboards::bishop_queen("e4"_b, ""_b) == "b1h1c2g2d3f3d5f5c6g6b7h7a8"_b);
static_assert(bitboards::pawn<WHITE>("e4"_b) == "d5f5"_b);
static_assert(bitboards::pawn<BLACK>("e4"_b) == "d3f3"_b);
