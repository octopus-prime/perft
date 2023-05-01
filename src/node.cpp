#include "node.hpp"

static_assert(sizeof(node) == 72, "node size is 72 bytes");
static_assert(node("e2"_b, 0, 0, 0, 0, 0, 0, 0, 0).occupied<WHITE>() == "e2"_b);
static_assert(node(0, "e4"_b, 0, 0, 0, 0, 0, 0, 0).occupied<BLACK>() == "e4"_b);
static_assert(node("e2"_b, "e4"_b, 0, 0, 0, 0, 0, 0, 0).occupied() == "e2e4"_b);
static_assert(node("e2"_b, 0, "e2"_b, 0, 0, 0, 0, 0, 0).king<WHITE>() == "e2"_b);
static_assert(node("e2"_b, 0, "e2"_b, 0, 0, 0, 0, 0, 0).attackers<WHITE>() == bitboards::king("e2"_b));
static_assert(node("e2"_b, 0, 0, "e2"_b, 0, 0, 0, 0, 0).rook_queen<WHITE>() == "e2"_b);
static_assert(node(0, "e2"_b, 0, "e2"_b, 0, 0, 0, 0, 0).attackers<BLACK>() == bitboards::rook_queen("e2"_b, ~0ull));
static_assert(node(0, "e2"_b, 0, 0, "e2"_b, 0, 0, 0, 0).bishop_queen<BLACK>() == "e2"_b);
static_assert(node(0, "e2"_b, 0, 0, "e2"_b, 0, 0, 0, 0).attackers<BLACK>() == bitboards::bishop_queen("e2"_b, ~0ull));
static_assert(node(0, "e2"_b, 0, 0, 0, "e2"_b, 0, 0, 0).knight<BLACK>() == "e2"_b);
static_assert(node(0, "e2"_b, 0, 0, 0, "e2"_b, 0, 0, 0).attackers<BLACK>() == bitboards::knight("e2"_b));

template <side_t side>
std::span<move> node::generate(std::span<move, 256> moves) const noexcept
{
    int index = 0;

    bitboard attacked = this->attackers<~side>();
    // bitboard checkers = this->checkers<side>();
    constexpr bitboard pinned = 0ull; // this->pinned<side>();
    constexpr bitboard valids = ~0ull;

    const auto generate_king = [&]() noexcept
    {
        square from = king<side>().find();
        bitboard targets = bitboards::king(from) & ~occupied<side>() & ~attacked;
        for (square to : targets)
            moves[index++] = {KING, from, to};
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
        bitboard sources = rook_queen<side>(); // & ~pinned;
        for (square from : sources)
        {
            piece moved = rook_or_queen[bishop_queen<side>()[from]];
            bitboard targets = bitboards::rook_queen(from, occupied()) & ~occupied<side>() & valids; // & valid_for_pinned[from]
            for (square to : targets)
                moves[index++] = {moved, from, to};
        }
    };

    const auto generate_bishop_queen = [&]() noexcept
    {
        constexpr piece bishop_or_queen[] = {BISHOP, QUEEN};
        bitboard sources = bishop_queen<side>(); // & ~pinned;
        for (square from : sources)
        {
            piece moved = bishop_or_queen[rook_queen<side>()[from]];
            bitboard targets = bitboards::bishop_queen(from, occupied()) & ~occupied<side>() & valids; // & valid_for_pinned[from]
            for (square to : targets)
                moves[index++] = {moved, from, to};
        }
    };

    const auto generate_pawn = [&]() noexcept
    {
        bitboard sources = pawn<side>();// & ~pinned;
        bitboard targets;

        if (side == WHITE) {
            targets = sources << 8 & ~occupied();    
            for (square to : targets)
                moves[index++] = {PAWN, to - 8, to};

            targets = targets << 8 & ~occupied() & "4"_r;
            for (square to : targets)
                moves[index++] = {PAWN, to - 16, to};

            targets = sources << 7 & ~"h"_f & occupied<~side>();
            for (square to : targets)
                moves[index++] = {PAWN, to - 7, to};

            targets = sources << 9 & ~"a"_f & occupied<~side>();
            for (square to : targets)
                moves[index++] = {PAWN, to - 9, to};
        } else {
            targets = sources >> 8 & ~occupied();    
            for (square to : targets)
                moves[index++] = {PAWN, to + 8, to};

            targets = targets >> 8 & ~occupied() & "5"_r;
            for (square to : targets)
                moves[index++] = {PAWN, to + 16, to};

            targets = sources >> 7 & ~"a"_f & occupied<~side>();
            for (square to : targets)
                moves[index++] = {PAWN, to + 7, to};

            targets = sources >> 9 & ~"h"_f & occupied<~side>();
            for (square to : targets)
                moves[index++] = {PAWN, to + 9, to};
        }
    };

    generate_king();
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
    const auto remove = [&](bitboard to) noexcept
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

    const auto execute_king = [&](bitboard from, bitboard to) noexcept
    {
        remove(to);
        king_.flip(bitboard{from | to});
        if (side == WHITE)
        {
            white.flip(bitboard{from | to});
            castle.reset("a1h1"_b);
        }
        else
        {
            black.flip(bitboard{from | to});
            castle.reset("a8h8"_b);
        }
    };

    const auto execute_knight = [&](bitboard from, bitboard to) noexcept
    {
        remove(to);
        knight_.flip(bitboard{from | to});
        if (side == WHITE)
        {
            white.flip(bitboard{from | to});
        }
        else
        {
            black.flip(bitboard{from | to});
        }
    };

    const auto execute_queen = [&](bitboard from, bitboard to) noexcept
    {
        remove(to);
        rook_queen_.flip(bitboard{from | to});
        bishop_queen_.flip(bitboard{from | to});
        if (side == WHITE)
        {
            white.flip(bitboard{from | to});
        }
        else
        {
            black.flip(bitboard{from | to});
        }
    };

    const auto execute_rook = [&](bitboard from, bitboard to) noexcept
    {
        remove(to);
        rook_queen_.flip(bitboard{from | to});
        if (side == WHITE)
        {
            white.flip(bitboard{from | to});
            castle.reset(bitboard{"a1h1"_b & from});
        }
        else
        {
            black.flip(bitboard{from | to});
            castle.reset(bitboard{"a8h8"_b & from});
        }
    };

    const auto execute_bishop = [&](bitboard from, bitboard to) noexcept
    {
        remove(to);
        bishop_queen_.flip(bitboard{from | to});
        if (side == WHITE)
        {
            white.flip(bitboard{from | to});
        }
        else
        {
            black.flip(bitboard{from | to});
        }
    };

    const auto execute_pawn = [&](bitboard from, bitboard to) noexcept
    {
        remove(to);
        pawn_.flip(bitboard{from | to});
        if (side == WHITE)
        {
            white.flip(bitboard{from | to});
        }
        else
        {
            black.flip(bitboard{from | to});
        }
    };

    switch (move.moved())
    {
    case KING:
        execute_king(bitboard{move.from()}, bitboard{move.to()});
        break;
    case KNIGHT:
        execute_knight(bitboard{move.from()}, bitboard{move.to()});
        break;
    case QUEEN:
        execute_queen(bitboard{move.from()}, bitboard{move.to()});
        break;
    case ROOK:
        execute_rook(bitboard{move.from()}, bitboard{move.to()});
        break;
    case BISHOP:
        execute_bishop(bitboard{move.from()}, bitboard{move.to()});
        break;
    case PAWN:
        execute_bishop(bitboard{move.from()}, bitboard{move.to()});
        break;
    }
}

template void node::execute<WHITE>(const move &move) noexcept;
template void node::execute<BLACK>(const move &move) noexcept;
