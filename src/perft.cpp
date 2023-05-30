#include "perft.hpp"
#include <chrono>
#include <format>
#include <iostream>

template <bool output>
template <side_t side, bool hashing, bool divide>
std::size_t perft_t<output>::perft(node &current, int depth) noexcept
{
    if (hashing)
    {
        auto hit = table_.get(current.hash<side>(), depth);
        if (hit.has_value())
        {
            return hit.value();
        }
    }
    std::size_t count = 0;
    std::array<move, 256> buffer;
    auto moves = current.generate<side, node::all>(buffer);
    for (const auto &move : moves)
    {
        std::size_t count_ = 0;
        if (depth <= 1)
            count_ += 1;
        else
        {
            node succ((node &)current);
            succ.execute<side>(move);
            std::array<struct move, 256> buffer2;
            count_ += depth == 2 ? succ.generate<~side, node::all>(buffer2).size() : perft<~side, hashing, false>(succ, depth - 1);
        }
        count += count_;
        if (divide) {
            std::cout << move << std::format("{:16L}", count_) << std::endl;
        }
    }
    if (hashing)
    {
        table_.put(current.hash<side>(), depth, count);
    }
    return count;
}

template <bool output>
void perft_t<output>::execute(node &current, side_t side, int depth)
{
    using as_floating_point = std::chrono::duration<double, std::ratio<1>>;
    auto time0 = std::chrono::high_resolution_clock::now();
    if (table_.empty()) {
        if (divide_)
            counter_ = side == WHITE ? perft<WHITE, false, true>(current, depth) : perft<BLACK, false, true>(current, depth);
        else
            counter_ = side == WHITE ? perft<WHITE, false, false>(current, depth) : perft<BLACK, false, false>(current, depth);
    }
    else {
        if (divide_)
            counter_ = side == WHITE ? perft<WHITE, true, true>(current, depth) : perft<BLACK, true, true>(current, depth);
        else
            counter_ = side == WHITE ? perft<WHITE, true, false>(current, depth) : perft<BLACK, true, false>(current, depth);
    }
    auto time1 = std::chrono::high_resolution_clock::now();
    auto time = duration_cast<as_floating_point>(time1 - time0).count();
    if (output) {
        std::cout << std::format("{:7.3f} {:16L} {:16L}", time, counter_, size_t(counter_ / time)) << std::endl;
    }
}

template void perft_t<false>::execute(node &current, side_t side, int depth);
template void perft_t<true>::execute(node &current, side_t side, int depth);
