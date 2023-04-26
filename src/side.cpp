#include "side.hpp"

static_assert(sizeof(side_t) == 1, "side_t size is 1 byte");
static_assert(~WHITE == BLACK, "not white is black");
static_assert(~BLACK == WHITE, "not black is white");
