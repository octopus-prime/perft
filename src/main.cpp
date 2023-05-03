#include "node.hpp"
#include <iostream>
#include <chrono>

template <side_t side>
std::size_t bar(const node &current, int depth) noexcept
{
  if (depth == 0)
  {
    return 1;
  }
  std::size_t count = 0;
  std::array<move, 256> buffer;
  auto moves = current.generate<side>(buffer);
  for (const auto &move : moves)
  {
    node succ(current);
    succ.execute<side>(move);
    if (!succ.checkers<side>())
      count += bar<~side>(succ, depth - 1);
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
      auto count = bar<WHITE>(current, 7);
      auto time1 = std::chrono::high_resolution_clock::now();
      auto time = duration_cast<as_floating_point>(time1 - time0).count();
      std::cout << std::format("n = {:L}", count) << std::endl;
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
