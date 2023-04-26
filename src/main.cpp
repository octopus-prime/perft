#include "bitboard.hpp"
#include "square.hpp"
#include <iostream>

int main() {
  constexpr auto e4_s = "e4"_s;
  constexpr auto e4_b = "e4"_b;
  try {
    std::cout << square_t{"b3"} << std::endl;
    std::cout << bitboard_t{"f6"_s} << std::endl;
    std::cout << bitboard_t{"f6f7f8"} << std::endl;
  } catch (const std::exception &exception) {
    std::cerr << exception.what() << std::endl;
  }
}
