#pragma once

#include "node.hpp"
#include "side.hpp"
#include "command.hpp"

class position {
  node root_;
  side_t side_;

public:
  position();
  position(std::string_view fen);

  void execute(Command auto& command, int depth) noexcept {
      command.execute(root_, side_, depth);
  }
};
