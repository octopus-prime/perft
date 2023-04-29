#include "bitboard.hpp"
#include <format>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <ranges>

std::ostream& operator<<(std::ostream& stream, bitboard bitboard) {
  char buffer[64];
  std::format_to(buffer, "{:064b}", (std::uint64_t) bitboard);
  for (auto row : buffer | std::views::chunk(8)) {
    std::ranges::reverse_copy(row, std::ostream_iterator<char>(stream));
    stream << std::endl;
  }
  return stream;
}

static_assert(sizeof(bitboard) == 8, "bitboard size is 8 bytes");
static_assert("b3f7"_b.empty() == false, "bitboard b3f7 is not empty");
static_assert("b3f7"_b.find() == "b3"_s, "bitboard b3f7 finds b3 first");
static_assert("b3f7"_b.count() == 2, "bitboard b3f7 has 2 bits set");
static_assert(bitboard("b"_f & "3"_r).find() == "b3"_s, "bitboard b3 is intersection of file b and rank 3");
