#include "position.hpp"
#include "test.hpp"
#include <iostream>
#include <chrono>

std::size_t test() {
  return test::run("../epd/perft_long.txt", 5);
}

std::size_t run() {
    const position position;
    return position.perft_divide(7);
}

int main()
{
  using as_floating_point = std::chrono::duration<double, std::ratio<1>>;
  try
  {
    std::locale::global(std::locale("en_US.UTF-8"));
    auto time0 = std::chrono::high_resolution_clock::now();
    // auto count = test();
    auto count = run();
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
