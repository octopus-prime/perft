#pragma once

#include "node.hpp"
#include <concepts>

template<typename T>
concept Command = requires(T command, node& current, side_t side, int depth)
{
    { command.execute(current, side, depth) } -> std::same_as<void>;
};
