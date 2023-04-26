#include "square.hpp"

std::ostream& operator<<(std::ostream& stream, square_t square) {
  return stream << char('a' + square.file()) << char('1' + square.rank());
}

static_assert(sizeof(square_t) == 1, "square_t size is 1 byte");
static_assert(square_t{"a1"} == 0, "square a1 has index 0");
static_assert(square_t{"h8"} == 63, "square h8 has index 63");
static_assert(square_t{"b3"}.file() == 1, "square b3 has file 1");
static_assert(square_t{"b3"}.rank() == 2, "square b3 has rank 2");
static_assert(square_t{"b3"} == "b3"_s, "square b3 from literal");
