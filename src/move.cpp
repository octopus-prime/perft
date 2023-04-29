#include "move.hpp"

static_assert(sizeof(move) == 2, "move should be 2 bytes");
static_assert(move{"e2"_s, "e4"_s}.from() == "e2"_s);
static_assert(move{"e2"_s, "e4"_s}.to() == "e4"_s);
