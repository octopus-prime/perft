#include "position.hpp"
#include "test.hpp"
#include <iostream>
#include <chrono>

int main()
{
  using as_floating_point = std::chrono::duration<double, std::ratio<1>>;
  try
  {
    std::locale::global(std::locale("en_US.UTF-8"));
    // const position position{"1rbq2n1/p2p2p1/P1pbp3/5rk1/PBpP2Pp/7P/R3R2N/1NQ1K3 b - d3 0 1"};
    // const position position{"rb6/5b2/1p2r3/p1k1P3/PpPPp3/2R4P/8/1N1K2R1 b - d3 0 1"};
    // const position position;
    // const position position{"4k3/8/5R1n/1p2rBp1/K3b3/2pp2P1/7P/1R4N1 w - b6 0 1"};

    const position position{"rb6/5b2/1pr5/p3P3/PpkPp1R1/7P/8/1N1K4 b - d3 0 1"};

    // for (int i = 1; i <= 7; ++i) {
    auto time0 = std::chrono::high_resolution_clock::now();
    // auto [count, checks] = position.perft_bulk(7);
    // auto count = position.perft_bulk(1);
    // auto count = position.perft_divide(1);
    auto count = test::run("epd/perft_long.txt", 6);
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
