#include "position.hpp"
#include "test.hpp"
#include <iostream>
#include <chrono>

int main(int argc, char *argv[])
{
  using as_floating_point = std::chrono::duration<double, std::ratio<1>>;

  if (argc != 3 && argc != 4)
  {
    std::cerr << "Usage: " << argv[0] << " <command> <depth> [<fen>]" << std::endl;
    std::cerr << "Commands: " << "p[erft] | d[ivide] | t[est]" << std::endl;
    return 1;
  }

  char command = argv[1][0];
  std::size_t depth = std::atoll(argv[2]);
  std::string_view fen = argc == 4 ? argv[3] : "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

  try
  {
    std::locale::global(std::locale("en_US.UTF-8"));

    // table table(15485863);
   table table(104395303);
// table table(512927377);

    auto time0 = std::chrono::high_resolution_clock::now();
    std::size_t count;
    switch (command) {
      case 'd':
        count = position{fen}.divide(table, depth);
        break;
      case 'p':
        count = position{fen}.perft(table, depth);
        break;
      case 't':
        count = test::run("../epd/perft_long.txt", depth);
        break;
      default:
        throw std::runtime_error("Unknown command");
    }
    auto time1 = std::chrono::high_resolution_clock::now();
    auto time = duration_cast<as_floating_point>(time1 - time0).count();
    std::cout << std::format("n = {:L}", count) << std::endl;
    std::cout << "t = " << time << std::endl;
    std::cout << std::format("p = {:L}", size_t(count / time)) << std::endl;
    std::cout << std::endl;
  }
  catch (const std::exception &exception)
  {
    std::cerr << exception.what() << std::endl;
  }
}
