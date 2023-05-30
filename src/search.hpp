#pragma once

#include "command.hpp"
#include "history.hpp"
#include "transposition.hpp"

class search_t {
    std::size_t counter;
    history_t history;
    transposition_t transposition;

    template <side_t side>
    int search(node& current, int alpha, int beta) noexcept;

    template <side_t side>
    std::pair<int, move> search(node &current, int alpha, int beta, int depth) noexcept;

public:
    search_t(std::size_t size) : counter{0}, history{}, transposition {size} {}

    void execute(node& current, side_t side, int depth);
};

static_assert(Command<search_t>);