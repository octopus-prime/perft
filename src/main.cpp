#include "node.hpp"
#include <iostream>
#include <chrono>

template <side_t side>
std::tuple<std::size_t, std::size_t> perft(const node &current, int depth) noexcept
{
  std::size_t count = 0;
  std::size_t checks = 0;
  std::array<move, 256> buffer;
  auto moves = current.generate<side>(buffer);
  for (const auto &move : moves)
  {
    node succ(current);
    succ.execute<side>(move);
    if (!succ.checkers<side>())
      if (depth > 1)
      {
        auto [count_, checks_] = perft<~side>(succ, depth - 1);
        count += count_;
        checks += checks_;
      }
      else
        ++count;
    else
      ++checks;
  }
  return {count, checks};
}

template <side_t side>
std::size_t perft_bulk(const node &current, int depth) noexcept
{
  std::size_t count = 0;
  std::array<move, 256> buffer;
  auto moves = current.generate<side>(buffer);
  for (const auto &move : moves)
  {
    if (depth <= 1)
      count += 1;
    else
    {
      node succ(current);
      succ.execute<side>(move);
      std::array<struct move, 256> buffer2;
      count += depth == 2 ? succ.generate<~side>(buffer2).size() : perft_bulk<~side>(succ, depth - 1);
    }
  }
  return count;
}

int main()
{
  using as_floating_point = std::chrono::duration<double, std::ratio<1>>;
  try
  {
    std::locale::global(std::locale("en_US.UTF-8"));
    constexpr node current;
    // for (int i = 1; i <= 7; ++i) {
    auto time0 = std::chrono::high_resolution_clock::now();
//    auto [count, checks] = perft_bulk<WHITE>(current, 7);
    auto count = perft_bulk<WHITE>(current, 8);
    auto checks = 0;
    auto time1 = std::chrono::high_resolution_clock::now();
    auto time = duration_cast<as_floating_point>(time1 - time0).count();
    std::cout << std::format("n = {:L}", count) << std::endl;
    std::cout << std::format("c = {:L}", checks) << std::endl;
    std::cout << "t = " << time << std::endl;
    std::cout << std::format("p = {:L}", size_t(count / time)) << std::endl;
    std::cout << std::endl;
    // }
  }
  catch (const std::exception &exception)
  {
    std::cerr << exception.what() << std::endl;
  }
}
