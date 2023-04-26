#pragma once

#include "square.hpp"
#include "side.hpp"
#include <bit>
#include <numeric>
#include <ranges>
#include <utility>
#include <vector>
#include <array>
#include <immintrin.h> // _pext
#include <algorithm>
#include <functional>

class bitboard_t
{
  using value_t = std::uint64_t;

  class iterator_t;
  class lookup_t;

  value_t _value;

public:
  constexpr bitboard_t(value_t _value = 0ull) noexcept
      : _value{_value} {}

  constexpr explicit bitboard_t(square_t square) noexcept
      : bitboard_t{1ull << square} {}

  constexpr bitboard_t(std::string_view string)
      : bitboard_t{parse(string)} {}

  constexpr operator value_t() const noexcept
  {
    return _value;
  }

  constexpr bool empty() const noexcept
  {
    return !_value;
  }

  constexpr auto count() const noexcept
  {
    return std::popcount(_value);
  }

  constexpr square_t find() const noexcept
  {
    return std::countr_zero(_value);
  }

  constexpr square_t pop() noexcept
  {
    square_t square = find();
    flip(square);
    return square;
  }

  constexpr void set(square_t square) noexcept
  {
    _value |= square;
  }

  constexpr void set(bitboard_t squares) noexcept
  {
    _value |= squares;
  }

  constexpr void reset(square_t square) noexcept
  {
    _value &= ~square;
  }

  constexpr void reset(bitboard_t squares) noexcept
  {
    _value &= ~squares;
  }

  constexpr void flip(square_t square) noexcept
  {
    _value ^= square;
  }

  constexpr void flip(bitboard_t squares) noexcept
  {
    _value ^= squares;
  }

  constexpr iterator_t begin() const noexcept;

  constexpr iterator_t end() const noexcept;

  static constexpr bitboard_t
  parse(std::string_view string)
  {
    auto view = string | std::views::chunk(2);
    return std::transform_reduce(view.begin(), view.end(), 0ull, std::bit_or{}, [](auto &&chunk)
                                 { return bitboard_t{square_t{std::string_view{chunk.data(), chunk.size()}}}; });
  }
};

class bitboard_t::iterator_t
{
  bitboard_t _value;

public:
  constexpr iterator_t(bitboard_t value) noexcept
      : _value(value) {}

  constexpr void operator++() const noexcept {}

  constexpr bool operator!=(iterator_t) const noexcept
  {
    return !_value.empty();
  }

  constexpr square_t operator*() noexcept
  {
    return _value.pop();
  }
};

constexpr bitboard_t::iterator_t bitboard_t::begin() const noexcept
{
  return {_value};
}

constexpr bitboard_t::iterator_t bitboard_t::end() const noexcept
{
  return {0ull};
}

class bitboard_t::lookup_t
{
  struct block_t
  {
    bitboard_t mask;
    std::vector<bitboard_t> data;
  };

  std::array<block_t, 64> blocks;

public:
//    constexpr
  bitboard_t
  operator()(square_t square, bitboard_t occupied) const noexcept
  {
    const auto &block = blocks[square];
    const auto index = _pext_u64(occupied, block.mask);
    return block.data[index];
  }
};

constexpr bitboard_t operator""_b(const char *data, size_t length)
{
  return std::string_view{data, length};
}

constexpr bitboard_t operator""_f(const char* data, size_t length) noexcept {
  constexpr auto mask = 0b100000001000000010000000100000001000000010000000100000001ull;
  const auto view = std::string_view{data, length};
  return std::transform_reduce(view.begin(), view.end(), 0ull, std::bit_or{}, [](char ch) {
    return mask << (ch - 'a');
  });
}

constexpr bitboard_t operator""_r(const char* data, size_t length) noexcept {
  constexpr auto mask = 0b11111111ull;
  const auto view = std::string_view{data, length};
  return std::transform_reduce(view.begin(), view.end(), 0ull, std::bit_or{}, [](char ch) {
    return mask << (ch - '1') * 8;
  });
}

std::ostream &operator<<(std::ostream &stream, bitboard_t bitboard);
