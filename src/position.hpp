#pragma once

#include "node.hpp"
#include "side.hpp"
#include "table.hpp"

class position {
  node root_;
  side_t side_;

  template <side_t side>
  static std::size_t perft(const node &current, int depth) noexcept;

  template <side_t side>
  static std::size_t divide(const node &current, int depth) noexcept;

  template <side_t side>
  static std::size_t perft(const node &current, table& table, int depth) noexcept;

  template <side_t side>
  static std::size_t divide(const node &current, table& table, int depth) noexcept;

  template <side_t side>
  static int search(node& current, int alpha, int beta) noexcept;

  template <side_t side>
  static std::pair<int, move> search(node &current, int alpha, int beta, int depth) noexcept;

public:
  position();
  position(std::string_view fen);

  std::size_t perft(int depth) const noexcept;
  std::size_t divide(int depth) const noexcept;

  std::size_t perft(table& table, int depth) const noexcept;
  std::size_t divide(table& table, int depth) const noexcept;

  int evaluate() noexcept;
  std::pair<int, move> search(int depth) noexcept;
};
