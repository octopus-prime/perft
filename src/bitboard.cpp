#include "bitboard.hpp"
#include <format>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <ranges>

// #include "literal.hpp"
// //
// //constexpr bitboard expand(bitboard in, auto&& view) noexcept {
// //  auto expand = [in](auto shift, auto mask_left, auto mask_right) noexcept {
// //    bitboard left = (in << shift) & mask_left;
// //    bitboard right = (in >> shift) & mask_right;
// //    return left | right;
// //  };
// //  return std::transform_reduce(view.begin(), view.end(), 0ull, std::bit_or{}, [expand](auto&& tuple) noexcept {
// //    return std::apply(expand, tuple);
// //  });
// //}
// //
// //constexpr bitboard expand_kings(bitboard in) noexcept {
// //  constexpr int shifts[] = {1, 8, 7, 9};
// //  constexpr uint64_t masks_left[] = {~"a"_f, ~""_f, ~"h"_f, ~"a"_f};
// //  constexpr uint64_t masks_right[] = {~"h"_f, ~""_f, ~"a"_f, ~"h"_f};
// //  return expand(in, std::views::zip(shifts, masks_left, masks_right));
// //}
// //
// //constexpr bitboard expand_knights(bitboard in) noexcept {
// //  constexpr int shifts[] = {10, 17, 15, 6};
// //  constexpr uint64_t masks_left[] = {~"ab"_f, ~"a"_f, ~"h"_f, ~"gh"_f};
// //  constexpr uint64_t masks_right[] = {~"gh"_f, ~"h"_f, ~"a"_f, ~"ab"_f};
// //  return expand(in, std::views::zip(shifts, masks_left, masks_right));
// //}
// //
// //constexpr std::array<bitboard, 64> as_array(auto&& expand) noexcept {
// //  auto view = std::views::iota(0, 64) | std::views::transform(expand);
// //  std::array<bitboard, 64> array;
// //  std::ranges::copy(view, array.begin());
// //  return array;
// //}
// //
// ////constinit std::array<bitboard, 64> bitboard::lookup_kings = as_array(expand_kings);

// constexpr std::array<bitboard, 64> bitboard::expand_leaper(auto&& expand) noexcept {
//   auto view = std::views::iota(0, 64) | std::views::transform(expand);
//   std::array<bitboard, 64> array;
//   std::ranges::copy(view, array.begin());
//   return array;
// }

// constinit std::array<bitboard, 64> bitboard::lookup_kings = bitboard::expand_leaper(bitboard::expand_kings);
// constinit std::array<bitboard, 64> bitboard::lookup_knights = bitboard::expand_leaper(bitboard::expand_knights);

std::ostream& operator<<(std::ostream& stream, bitboard_t bitboard) {
  char buffer[64];
  std::format_to(buffer, "{:064b}", (std::uint64_t) bitboard);
  for (auto row : buffer | std::views::chunk(8)) {
    std::ranges::reverse_copy(row, std::ostream_iterator<char>(stream));
    stream << std::endl;
  }
  return stream;
}

static_assert(sizeof(bitboard_t) == 8, "bitboard_t size is 8 bytes");
static_assert("b3f7"_b.empty() == false, "bitboard b3f7 is not empty");
static_assert("b3f7"_b.find() == "b3"_s, "bitboard b3f7 finds b3 first");
static_assert("b3f7"_b.count() == 2, "bitboard b3f7 has 2 bits set");
