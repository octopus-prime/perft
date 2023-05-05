#include "position.hpp"
#include <iostream>
#include <chrono>

int main()
{
  using as_floating_point = std::chrono::duration<double, std::ratio<1>>;
  try
  {
    std::locale::global(std::locale("en_US.UTF-8"));
    const position position;
    // for (int i = 1; i <= 7; ++i) {
    auto time0 = std::chrono::high_resolution_clock::now();
//    auto [count, checks] = perft_bulk<WHITE>(current, 7);
    auto count = position.perft_bulk(7);
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
