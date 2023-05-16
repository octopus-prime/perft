#pragma once

#include "position.hpp"
#include <expected>

class test {
  position position_;
  std::array<std::size_t, 6> expected_;
  std::size_t index_;

  test(std::string_view epd, std::size_t index);
  std::expected<std::size_t, std::string> run(int depth) const noexcept;

public:
  static std::size_t run(std::string_view file, int depth);
};
