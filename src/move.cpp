#include "move.hpp"

static_assert(sizeof(move) == 4, "move should be 4 bytes");
static_assert(move{PAWN, "e2"_s, "e4"_s}.from() == "e2"_s);
static_assert(move{PAWN, "e2"_s, "e4"_s}.to() == "e4"_s);
static_assert(move{PAWN, "e2"_s, "e4"_s}.moved() == PAWN);
static_assert(move{PAWN, "e2"_s, "e4"_s}.promoted() == EMPTY);

std::ostream& operator<<(std::ostream& stream, move move) {
    return stream << move.from() << move.to();
}
