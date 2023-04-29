#pragma once

#include <cstdint>
#include <string_view>
#include <ostream>

class square {
  using value__t = std::uint8_t;

  value__t value_;

public:
  constexpr square(std::integral auto value) noexcept
      : value_{std::forward<value__t>(value)} {}

  constexpr square(std::integral auto file, std::integral auto rank) noexcept
      : square{8 * rank + file} {}

  constexpr square(std::string_view string)
      : square{parse(string)} {
  }
  
  constexpr operator value__t() const noexcept {
    return value_;
  }

  constexpr auto file() const noexcept {
    return value_ % 8;
  }

  constexpr auto rank() const noexcept {
    return value_ / 8;
  }

  constexpr void operator++() noexcept {
    ++value_;
  }

  constexpr void operator+=(std::integral auto value) noexcept {
    value_ += value;
  }
  
  static constexpr square parse(std::string_view string) {
    return {string.at(0) - 'a', string.at(1) - '1'};
  }
};

constexpr square operator""_s(const char* data, size_t length) {
  return std::string_view{data, length};
}

std::ostream& operator<<(std::ostream& stream, square square);
