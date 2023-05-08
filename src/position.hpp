#pragma once

#include "node.hpp"
#include "side.hpp"

class position {
  node root_;
  side_t side_;

  template <side_t side>
  static std::size_t perft(const node &current, int depth) noexcept;

  template <side_t side>
  static std::size_t divide(const node &current, int depth) noexcept;

public:
  position();
  position(std::string_view fen);

  std::size_t perft(int depth) const noexcept;
  std::size_t divide(int depth) const noexcept;
};
