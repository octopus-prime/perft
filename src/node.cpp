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
    bitboard checkers = this->checkers<side>();
    bitboard pinned = 0ull; // this->pinned<side>();
    bitboard valids = ~0ull;

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
        bitboard sources = rook_queen<side>();// & ~pinned;
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
        bitboard sources = bishop_queen<side>();// & ~pinned;
        for (square from : sources)
        {
            piece moved = bishop_or_queen[rook_queen<side>()[from]];
            bitboard targets = bitboards::bishop_queen(from, occupied()) & ~occupied<side>() & valids; // & valid_for_pinned[from]
            for (square to : targets)
                moves[index++] = {moved, from, to};
        }
    };

    generate_king();
    generate_knight();
    generate_rook_queen();
    generate_bishop_queen();

    return moves.subspan(0, index);
}

template std::span<move> node::generate<WHITE>(std::span<move, 256> moves) const noexcept;
template std::span<move> node::generate<BLACK>(std::span<move, 256> moves) const noexcept;

template <side_t side>
void node::execute(const move &move) noexcept
{
}

template void node::execute<WHITE>(const move &move) noexcept;
template void node::execute<BLACK>(const move &move) noexcept;
