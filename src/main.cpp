#include "node.hpp"
#include <iostream>

void foo() {
    constexpr node current;
    std::array<move, 256> buffer;
    auto moves = current.generate<WHITE>(buffer);
    for (const auto& move : moves) {
        std::cout << move << std::endl;
    }
}

int main() {
  constexpr auto e4_s = "e4"_s;
  constexpr auto e4_b = "e4"_b;
  const auto x = bitboards::knight(e4_s);
  constexpr auto y = bitboards::knight(e4_b);
  const auto z = bitboards::line("c3"_s, "f6"_s);
  try {
    std::cout << square{"b3"} << std::endl;
    std::cout << bitboard{"f6"_s} << std::endl;
    std::cout << bitboard{"f6f7f8"} << std::endl;
    std::cout << x << std::endl;
    std::cout << y << std::endl;
    std::cout << z << std::endl;
    foo();
  } catch (const std::exception &exception) {
    std::cerr << exception.what() << std::endl;
  }
}
