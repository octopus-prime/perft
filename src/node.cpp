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
std::span<move> node::generate(std::span<move, 256> moves) const noexcept {
    return moves.subspan(0, 0);
}

template std::span<move> node::generate<WHITE>(std::span<move, 256> moves) const noexcept;
template std::span<move> node::generate<BLACK>(std::span<move, 256> moves) const noexcept;

template <side_t side>
void node::execute(const move& move) noexcept {
}

template void node::execute<WHITE>(const move& move) noexcept;
template void node::execute<BLACK>(const move& move) noexcept;
