#include "node.hpp"
#include <utility>

static_assert(sizeof(node) == 72, "node size is 72 bytes");
static_assert(node("e2"_b, 0, 0, 0, 0, 0, 0, 0, 0).occupied<WHITE>() == "e2"_b);
static_assert(node(0, "e4"_b, 0, 0, 0, 0, 0, 0, 0).occupied<BLACK>() == "e4"_b);
static_assert(node("e2"_b, "e4"_b, 0, 0, 0, 0, 0, 0, 0).occupied() == "e2e4"_b);
static_assert(node("e2"_b, 0, "e2"_b, 0, 0, 0, 0, 0, 0).king<WHITE>() == "e2"_b);
static_assert(node("e2"_b, 0, "e2"_b, 0, 0, 0, 0, 0, 0).attackers<WHITE>() == bitboards::king("e2"_b));
static_assert(node("e2"_b, 0, 0, "e2"_b, 0, 0, 0, 0, 0).rook_queen<WHITE>() == "e2"_b);
static_assert(node(0, "e2"_b, 0, "e2"_b, 0, 0, 0, 0, 0).attackers<BLACK>() == bitboards::rook_queen("e2"_b, 0ull));
static_assert(node(0, "e2"_b, 0, 0, "e2"_b, 0, 0, 0, 0).bishop_queen<BLACK>() == "e2"_b);
static_assert(node(0, "e2"_b, 0, 0, "e2"_b, 0, 0, 0, 0).attackers<BLACK>() == bitboards::bishop_queen("e2"_b, 0ull));
static_assert(node(0, "e2"_b, 0, 0, 0, "e2"_b, 0, 0, 0).knight<BLACK>() == "e2"_b);
static_assert(node(0, "e2"_b, 0, 0, 0, "e2"_b, 0, 0, 0).attackers<BLACK>() == bitboards::knight("e2"_b));

template <side_t side>
std::span<move> node::generate(std::span<move, 256> moves) const noexcept
{
    int index = 0;

    bitboard attacked = this->attackers<~side>();
    bitboard checkers = this->checkers<side>();
    bitboard valids = ~0ull;
    bitboard validse = 0ull;
    bitboard pinned = 0ull;
    bitboard valid_for_pinned[64];
    std::fill_n(valid_for_pinned, 64, ~0ull);

    const auto generate_pinned = [&]() noexcept
    {
        square ksq = king<side>().find();
        bitboard ksr = bitboards::rook_queen(ksq, occupied()) & occupied<side>();
        bitboard ksb = bitboards::bishop_queen(ksq, occupied()) & occupied<side>();
        bitboard bcr = bitboards::rook_queen(ksq, 0ull) & rook_queen<~side>();
        bitboard bcb = bitboards::bishop_queen(ksq, 0ull) & bishop_queen<~side>();
        for (square sq : bcr)
        {
            bitboard rsk = bitboards::rook_queen(sq, occupied()) & occupied<side>();
            bitboard pin = (ksr & rsk);
            pinned |= pin;
            if (pin)
                valid_for_pinned[pin.find()] = bitboards::line(ksq, sq);
        }
        for (square sq : bcb)
        {
            bitboard bsk = bitboards::bishop_queen(sq, occupied()) & occupied<side>();
            bitboard pin = (ksb & bsk);
            pinned |= pin;
            if (pin)
                valid_for_pinned[pin.find()] = bitboards::line(ksq, sq);
        }
    };

    const auto generate_king = [&]() noexcept
    {
        square from = king<side>().find();
        bitboard targets = bitboards::king(from) & ~occupied<side>() & ~attacked;
        for (square to : targets)
            moves[index++] = {KING, from, to};
    };

    const auto generate_castle = [&]() noexcept
    {
        if (side == WHITE)
        {
            if ((castle & "h1"_b) && !(occupied() & "f1g1"_b) && !(attacked & "e1f1g1"_b))
                moves[index++] = {KING, "e1"_s, "g1"_s, CASTLE_SHORT};
            if ((castle & "a1"_b) && !(occupied() & "b1c1d1"_b) && !(attacked & "e1d1c1"_b))
                moves[index++] = {KING, "e1"_s, "c1"_s, CASTLE_LONG};
        }
        else
        {
            if ((castle & "h8"_b) && !(occupied() & "f8g8"_b) && !(attacked & "e8f8g8"_b))
                moves[index++] = {KING, "e8"_s, "g8"_s, CASTLE_SHORT};
            if ((castle & "a8"_b) && !(occupied() & "b8c8d8"_b) && !(attacked & "e8d8c8"_b))
                moves[index++] = {KING, "e8"_s, "c8"_s, CASTLE_LONG};
        }
    };

    const auto generate_knight = [&]() noexcept
    {
        bitboard sources = knight<side>() & ~pinned;
        for (square from : sources)
        {
            bitboard targets = bitboards::knight(from) & ~occupied<side>() & valids;
            for (square to : targets)
                moves[index++] = {KNIGHT, from, to};
        }
    };

    const auto generate_rook_queen = [&]() noexcept
    {
        constexpr piece rook_or_queen[] = {ROOK, QUEEN};
        bitboard sources = rook_queen<side>();
        for (square from : sources)
        {
            piece moved = rook_or_queen[bishop_queen<side>()[from]];
            bitboard targets = bitboards::rook_queen(from, occupied()) & ~occupied<side>() & valids & valid_for_pinned[from];
            for (square to : targets)
                moves[index++] = {moved, from, to};
        }
    };

    const auto generate_bishop_queen = [&]() noexcept
    {
        constexpr piece bishop_or_queen[] = {BISHOP, QUEEN};
        bitboard sources = bishop_queen<side>();
        for (square from : sources)
        {
            piece moved = bishop_or_queen[rook_queen<side>()[from]];
            bitboard targets = bitboards::bishop_queen(from, occupied()) & ~occupied<side>() & valids & valid_for_pinned[from];
            for (square to : targets)
                moves[index++] = {moved, from, to};
        }
    };

    const auto generate_pawn = [&]() noexcept
    {
        const auto generate_normal = [&](bitboard targets, int delta) noexcept
        {
            for (square to : targets)
                if (valid_for_pinned[to + delta] & bitboard{to})
                    moves[index++] = {PAWN, to + delta, to};
        };

        const auto generate_double_push = [&](bitboard targets, int delta) noexcept
        {
            for (square to : targets)
                if (valid_for_pinned[to + delta] & bitboard{to})
                    moves[index++] = {PAWN, to + delta, to, DOUBLE_PUSH};
        };

        const auto generate_promotion = [&](bitboard targets, int delta) noexcept
        {
            for (square to : targets)
                if (valid_for_pinned[to + delta] & bitboard{to})
                    for (special promotion : {PROMOTE_QUEEN, PROMOTE_ROOK, PROMOTE_BISHOP, PROMOTE_KNIGHT})
                        moves[index++] = {PAWN, to + delta, to, promotion};
        };

        const bitboard sources = pawn<side>(); // & ~pinned;
        bitboard targets;

        if (side == WHITE)
        {
            const bitboard push = sources << 8 & ~occupied();

            targets = push & valids;
            generate_normal(targets & ~"8"_r, -8);
            generate_promotion(targets & "8"_r, -8);

            targets = push << 8 & ~occupied() & "4"_r & valids;
            generate_double_push(targets, -16);

            targets = sources << 7 & ~"h"_f & occupied<~side>() & valids;
            generate_normal(targets & ~"8"_r, -7);
            generate_promotion(targets & "8"_r, -7);

            targets = sources << 9 & ~"a"_f & occupied<~side>() & valids;
            generate_normal(targets & ~"8"_r, -9);
            generate_promotion(targets & "8"_r, -9);
        }
        else
        {
            const bitboard push = sources >> 8 & ~occupied();

            targets = push & valids;
            generate_normal(targets & ~"1"_r, +8);
            generate_promotion(targets & "1"_r, +8);

            targets = push >> 8 & ~occupied() & "5"_r & valids;
            generate_double_push(targets, +16);

            targets = sources >> 7 & ~"a"_f & occupied<~side>() & valids;
            generate_normal(targets & ~"1"_r, +7);
            generate_promotion(targets & "1"_r, +7);

            targets = sources >> 9 & ~"h"_f & occupied<~side>() & valids;
            generate_normal(targets & ~"1"_r, +9);
            generate_promotion(targets & "1"_r, +9);
        }

        bitboard board = sources & bitboards::pawn<~side>(bitboard{en_passant & (valids | validse)});
        for (square from : board)
            if (valid_for_pinned[from] & en_passant)
            {
                square ksq = king<side>().find();
                square esq = en_passant.find();
                square psq = side == WHITE ? esq - 8 : esq + 8;
                if (ksq.rank() == psq.rank())
                {
                    bitboard occ = occupied() & ~(bitboard{psq} | bitboard{from});
                    bitboard foo = bitboards::rook_queen(ksq, occ) & rook_queen<~side>();
                    if (foo)
                        continue;
                }
                moves[index++] = {PAWN, from, esq, EN_PASSANT};
            }
    };

    generate_king();

    switch (checkers.count())
    {
    case 0:
        [[likely]] generate_castle();
        break;
    case 1:
    {
        square from = king<side>().find();
        square to = checkers.find();
        valids = bitboards::line(from, to);
        if (side == BLACK)
        {
            if (checkers == en_passant << 8)
                validse = en_passant;
        }
        else
        {
            if (checkers == en_passant >> 8)
                validse = en_passant;
        }
    }
    break;
    default:
        return moves.subspan(0, index);
    }

    generate_pinned();

    generate_knight();
    generate_rook_queen();
    generate_bishop_queen();
    generate_pawn();

    return moves.subspan(0, index);
}

template std::span<move> node::generate<WHITE>(std::span<move, 256> moves) const noexcept;
template std::span<move> node::generate<BLACK>(std::span<move, 256> moves) const noexcept;

template <side_t side>
void node::execute(const move &move) noexcept
{
    const auto remove = [this](bitboard to) noexcept
    {
        rook_queen_.reset(to);
        bishop_queen_.reset(to);
        knight_.reset(to);
        pawn_.reset(to);
        if (side == WHITE)
        {
            black.reset(to);
            castle.reset(bitboard{"a8h8"_b & to});
        }
        else
        {
            white.reset(to);
            castle.reset(bitboard{"a1h1"_b & to});
        }
    };

    const bitboard from{move.from()};
    const bitboard to{move.to()};
    const bitboard squares{from | to};
    en_passant = 0ull;

    const auto execute_king = [&]() noexcept
    {
        remove(to);
        king_.flip(squares);
        if (side == WHITE)
        {
            white.flip(squares);
            castle.reset("a1h1"_b);
        }
        else
        {
            black.flip(squares);
            castle.reset("a8h8"_b);
        }
    };

    const auto execute_castle_short = [&]() noexcept
    {
        if (side == WHITE)
        {
            king_.flip("e1g1"_b);
            rook_queen_.flip("h1f1"_b);
            white.flip("e1f1g1h1"_b);
            castle.reset("a1h1"_b);
        }
        else
        {
            king_.flip("e8g8"_b);
            rook_queen_.flip("h8f8"_b);
            black.flip("e8f8g8h8"_b);
            castle.reset("a8h8"_b);
        }
    };

    const auto execute_castle_long = [&]() noexcept
    {
        if (side == WHITE)
        {
            king_.flip("e1c1"_b);
            rook_queen_.flip("a1d1"_b);
            white.flip("a1c1d1e1"_b);
            castle.reset("a1h1"_b);
        }
        else
        {
            king_.flip("e8c8"_b);
            rook_queen_.flip("a8d8"_b);
            black.flip("a8c8d8e8"_b);
            castle.reset("a8h8"_b);
        }
    };

    const auto execute_knight = [&]() noexcept
    {
        remove(to);
        knight_.flip(squares);
        if (side == WHITE)
        {
            white.flip(squares);
        }
        else
        {
            black.flip(squares);
        }
    };

    const auto execute_queen = [&]() noexcept
    {
        remove(to);
        rook_queen_.flip(squares);
        bishop_queen_.flip(squares);
        if (side == WHITE)
        {
            white.flip(squares);
        }
        else
        {
            black.flip(squares);
        }
    };

    const auto execute_rook = [&]() noexcept
    {
        remove(to);
        rook_queen_.flip(squares);
        if (side == WHITE)
        {
            white.flip(squares);
            castle.reset(bitboard{"a1h1"_b & from});
        }
        else
        {
            black.flip(squares);
            castle.reset(bitboard{"a8h8"_b & from});
        }
    };

    const auto execute_bishop = [&]() noexcept
    {
        remove(to);
        bishop_queen_.flip(squares);
        if (side == WHITE)
        {
            white.flip(squares);
        }
        else
        {
            black.flip(squares);
        }
    };

    const auto execute_pawn = [&]() noexcept
    {
        remove(to);
        pawn_.flip(squares);
        if (side == WHITE)
        {
            white.flip(squares);
        }
        else
        {
            black.flip(squares);
        }
    };

    const auto execute_promote_queen = [&]() noexcept
    {
        remove(to);
        pawn_.flip(from);
        rook_queen_.flip(to);
        bishop_queen_.flip(to);
        if (side == WHITE)
        {
            white.flip(squares);
        }
        else
        {
            black.flip(squares);
        }
    };

    const auto execute_promote_rook = [&]() noexcept
    {
        remove(to);
        pawn_.flip(from);
        rook_queen_.flip(to);
        if (side == WHITE)
        {
            white.flip(squares);
        }
        else
        {
            black.flip(squares);
        }
    };

    const auto execute_promote_bishop = [&]() noexcept
    {
        remove(to);
        pawn_.flip(from);
        bishop_queen_.flip(to);
        if (side == WHITE)
        {
            white.flip(squares);
        }
        else
        {
            black.flip(squares);
        }
    };

    const auto execute_promote_knight = [&]() noexcept
    {
        remove(to);
        pawn_.flip(from);
        knight_.flip(to);
        if (side == WHITE)
        {
            white.flip(squares);
        }
        else
        {
            black.flip(squares);
        }
    };

    const auto execute_double_push = [&]() noexcept
    {
        pawn_.flip(squares);
        if (side == WHITE)
        {
            white.flip(squares);
            en_passant = to >> 8;
        }
        else
        {
            black.flip(squares);
            en_passant = to << 8;
        }
    };

    const auto execute_en_passant = [&]() noexcept
    {
        pawn_.flip(squares);
        if (side == WHITE)
        {
            white.flip(squares);
            remove(to >> 8);
        }
        else
        {
            black.flip(squares);
            remove(to << 8);
        }
    };

    switch (move.moved())
    {
    case KING:
        switch (move.flag())
        {
        case NONE:
            [[likely]] execute_king();
            break;
        case CASTLE_SHORT:
            execute_castle_short();
            break;
        case CASTLE_LONG:
            execute_castle_long();
            break;
        default:
            std::unreachable();
        }
        break;
    case KNIGHT:
        execute_knight();
        break;
    case QUEEN:
        execute_queen();
        break;
    case ROOK:
        execute_rook();
        break;
    case BISHOP:
        execute_bishop();
        break;
    case PAWN:
        switch (move.flag())
        {
        case NONE:
            [[likely]] execute_pawn();
            break;
        case PROMOTE_QUEEN:
            execute_promote_queen();
            break;
        case PROMOTE_ROOK:
            execute_promote_rook();
            break;
        case PROMOTE_BISHOP:
            execute_promote_bishop();
            break;
        case PROMOTE_KNIGHT:
            execute_promote_knight();
            break;
        case DOUBLE_PUSH:
            execute_double_push();
            break;
        case EN_PASSANT:
            execute_en_passant();
            break;
        default:
            std::unreachable();
        }
        break;
    }
}

template void node::execute<WHITE>(const move &move) noexcept;
template void node::execute<BLACK>(const move &move) noexcept;
