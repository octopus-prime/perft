#pragma once

#include <cstdint>
#include <string_view>
#include <ostream>

class square_t {
  using value_t = std::uint8_t;

  value_t _value;

public:
  constexpr square_t(std::integral auto value) noexcept
      : _value{std::forward<value_t>(value)} {}

  constexpr square_t(std::integral auto file, std::integral auto rank) noexcept
      : square_t{8 * rank + file} {}

  constexpr square_t(std::string_view string)
      : square_t{parse(string)} {
  }
  
  constexpr operator value_t() const noexcept {
    return _value;
  }

  constexpr auto file() const noexcept {
    return _value % 8;
  }

  constexpr auto rank() const noexcept {
    return _value / 8;
  }
  
  static constexpr square_t parse(std::string_view string) {
    return {string.at(0) - 'a', string.at(1) - '1'};
  }
};

constexpr square_t operator""_s(const char* data, size_t length) {
  return std::string_view{data, length};
}

std::ostream& operator<<(std::ostream& stream, square_t square);
