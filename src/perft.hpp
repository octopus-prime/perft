#pragma once

#include "command.hpp"
#include "table.hpp"

template <bool output>
class perft_t {
    std::size_t counter_;
    table table_;
    bool divide_;

    template <side_t side, bool hashing, bool divide>
    std::size_t perft(node &current, int depth) noexcept;

public:
    perft_t(bool divide) : counter_{0}, table_{}, divide_{divide} {}
    perft_t(bool divide, std::size_t size) : counter_{0}, table_{size}, divide_{divide} {}

    void execute(node& current, side_t side, int depth);

    std::size_t counter() const noexcept {
        return counter_;
    }
};

static_assert(Command<perft_t<false>>);
static_assert(Command<perft_t<true>>);
