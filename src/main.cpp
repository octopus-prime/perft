#include "node.hpp"
#include <iostream>
#include <chrono>

template <side_t side>
void foo(const node &current) noexcept
{
  std::array<move, 256> buffer;
  auto moves = current.generate<WHITE>(buffer);
  for (const auto &move : moves)
  {
    std::cout << move << std::endl;
    node succ(current);
    succ.execute<WHITE>(move);
  }
}

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

  constexpr auto e4_s = "e4"_s;
  constexpr auto e4_b = "e4"_b;
  const auto x = bitboards::knight(e4_s);
  constexpr auto y = bitboards::knight(e4_b);
  const auto z = bitboards::line("c3"_s, "f6"_s);
  try
  {
    std::locale::global(std::locale("de_DE.UTF-8"));

    std::cout << square{"b3"} << std::endl;
    std::cout << bitboard{"f6"_s} << std::endl;
    std::cout << bitboard{"f6f7f8"} << std::endl;
    std::cout << x << std::endl;
    std::cout << y << std::endl;
    std::cout << z << std::endl;

    constexpr node current;

    foo<WHITE>(current);

    auto time0 = std::chrono::high_resolution_clock::now();
    auto count = bar<WHITE>(current, 6);
    auto time1 = std::chrono::high_resolution_clock::now();
    auto time = duration_cast<as_floating_point>(time1 - time0).count();
    std::cout << "n = " << std::format("{:L}", count) << std::endl;
    std::cout << "t = " << time << std::endl;
    std::cout << "p = " << std::format("{:L}", size_t(count / time)) << std::endl;
    std::cout << std::endl;
  }
  catch (const std::exception &exception)
  {
    std::cerr << exception.what() << std::endl;
  }
}
