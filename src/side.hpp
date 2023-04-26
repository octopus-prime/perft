#pragma once

#include <cstdint>

enum side_t : std::uint8_t {WHITE, BLACK};

constexpr side_t operator~(side_t side) noexcept {
  return side == WHITE ? BLACK : WHITE;
}
