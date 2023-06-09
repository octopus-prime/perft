#pragma once

#include "square.hpp"
#include <bit>
#include <numeric>
#include <ranges>
#include <functional>

class bitboard
{
  using value_t = std::uint64_t;

  class iterator_t;

  value_t value_;

public:
  constexpr bitboard(value_t value = 0ull) noexcept
      : value_{value} {}

  constexpr explicit bitboard(square square) noexcept
      : bitboard  {1ull << square} {}

  constexpr bitboard(std::string_view string)
      : bitboard  {parse(string)} {}

  constexpr operator value_t() const noexcept
  {
    return value_;
  }

  constexpr bool empty() const noexcept
  {
    return value_ == 0ull;
  }

  constexpr auto count() const noexcept
  {
    return std::popcount(value_);
  }

  constexpr square find() const noexcept
  {
    return std::countr_zero(value_);
  }

  constexpr square pop() noexcept
  {
    square square = find();
    flip(square);
    return square;
  }

  constexpr void set(square square) noexcept
  {
    value_ |= bitboard{square};
  }

  constexpr void set(bitboard squares) noexcept
  {
    value_ |= squares;
  }

  constexpr void reset(square square) noexcept
  {
    value_ &= ~bitboard{square};
  }

  constexpr bitboard& reset(bitboard squares) noexcept
  {
    value_ &= ~squares;
    return *this;
  }

  constexpr void flip(square square) noexcept
  {
    value_ ^= bitboard{square};
  }

  constexpr void flip(bitboard squares) noexcept
  {
    value_ ^= squares;
  }

  constexpr bool operator[](square square) const noexcept {
    return value_ & bitboard{square};
  }

  constexpr void operator|=(bitboard squares) noexcept {
    value_ |= squares;
  }

  constexpr void operator&=(bitboard squares) noexcept {
    value_ &= squares;
  }

  constexpr void operator<<=(std::integral auto shift) noexcept {
    value_ <<= shift;
  }

  constexpr void operator>>=(std::integral auto shift) noexcept {
    value_ >>= shift;
  }

  constexpr iterator_t begin() const noexcept;

  constexpr iterator_t end() const noexcept;

  static constexpr bitboard
  parse(std::string_view string)
  {
    auto view = string | std::views::chunk(2);
    return std::transform_reduce(view.begin(), view.end(), 0ull, std::bit_or{}, [](auto &&chunk)
                                 { return bitboard
                              {square{std::string_view{chunk.data(), chunk.size()}}}; });
  }
};

class bitboard::iterator_t
{
  bitboard value_;

public:
  constexpr iterator_t(bitboard value) noexcept
      : value_(value) {}

  constexpr void operator++() const noexcept {}

  constexpr bool operator!=(iterator_t) const noexcept
  {
    return !value_.empty();
  }

  constexpr square operator*() noexcept
  {
    return value_.pop();
  }
};

constexpr bitboard::iterator_t bitboard::begin() const noexcept
{
  return {value_};
}

constexpr bitboard::iterator_t bitboard::end() const noexcept
{
  return {0ull};
}

constexpr bitboard operator""_b(const char *data, size_t length)
{
  return std::string_view{data, length};
}

constexpr bitboard operator""_f(const char *data, size_t length) noexcept
{
  constexpr auto mask = 0b100000001000000010000000100000001000000010000000100000001ull;
  const auto view = std::string_view{data, length};
  return std::transform_reduce(view.begin(), view.end(), 0ull, std::bit_or{}, [](char ch)
                               { return mask << (ch - 'a'); });
}

constexpr bitboard operator""_r(const char *data, size_t length) noexcept
{
  constexpr auto mask = 0b11111111ull;
  const auto view = std::string_view{data, length};
  return std::transform_reduce(view.begin(), view.end(), 0ull, std::bit_or{}, [](char ch)
                               { return mask << (ch - '1') * 8; });
}

std::ostream &operator<<(std::ostream &stream, bitboard bitboard);
