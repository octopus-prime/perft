#pragma once

#include <cstdint>
#include <string_view>
#include <ostream>

class square {
  using value_t = std::uint8_t;

  value_t value;

public:
  constexpr square(std::integral auto value) noexcept
      : value{std::forward<value_t>(value)} {}

  constexpr square(std::integral auto file, std::integral auto rank) noexcept
      : square{8 * rank + file} {}

  constexpr square(std::string_view string)
      : square{parse(string)} {
  }
  
  constexpr operator value_t() const noexcept {
    return value;
  }

  constexpr auto file() const noexcept {
    return value % 8;
  }

  constexpr auto rank() const noexcept {
    return value / 8;
  }

  constexpr void operator++() noexcept {
    ++value;
  }

  constexpr void operator+=(std::integral auto value) noexcept {
    value += value;
  }
  
  static constexpr square parse(std::string_view string) {
    return {string.at(0) - 'a', string.at(1) - '1'};
  }
};

constexpr square operator""_s(const char* data, size_t length) {
  return std::string_view{data, length};
}

std::ostream& operator<<(std::ostream& stream, square square);
