#include "move.hpp"

static_assert(sizeof(move) == 3, "move should be 3 bytes");
static_assert(move{move::PAWN, "e2"_s, "e4"_s}.from == "e2"_s);
static_assert(move{move::PAWN, "e2"_s, "e4"_s}.to == "e4"_s);
static_assert(move{move::PAWN, "e2"_s, "e4"_s}.type == move::PAWN);

std::ostream& operator<<(std::ostream& stream, const move& move) {
    switch (move.type)
    {
    case move::KING:
        stream << 'K' << move.from << move.to;
        break;
    case move::CASTLE_SHORT:
        stream << "O-O";
        break;
    case move::CASTLE_LONG:
        stream << "O-O-O";
        break;
    case move::KNIGHT:
        stream << 'N' << move.from << move.to;
        break;
    case move::QUEEN:
        stream << 'Q' << move.from << move.to;
        break;
    case move::ROOK:
        stream << 'R' << move.from << move.to;
        break;
    case move::BISHOP:
        stream << 'B' << move.from << move.to;
        break;
    case move::PAWN:
    case move::DOUBLE_PUSH:
        stream << 'P' << move.from << move.to;
        break;
    case move::PROMOTE_QUEEN:
        stream << 'P' << move.from << move.to << 'Q';
        break;
    case move::PROMOTE_ROOK:
        stream << 'P' << move.from << move.to << 'R';
        break;
    case move::PROMOTE_BISHOP:
        stream << 'P' << move.from << move.to << 'B';
        break;
    case move::PROMOTE_KNIGHT:
        stream << 'P' << move.from << move.to << 'N';
        break;
    case move::EN_PASSANT:
        stream << 'P' << move.from << move.to << "ep";
        break;
    }
    return stream;
}
