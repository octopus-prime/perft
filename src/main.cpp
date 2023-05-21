#include "position.hpp"
#include "test.hpp"
#include <iostream>
#include <chrono>

#include "nnue.hpp"

extern std::size_t counter;

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
  std::size_t size = 0;//command == 't' ? 0 :  15'485'863;
  // std::size_t size = command == 't' ? 0 : 104'395'303;
  // std::size_t size = command == 't' ? 0 : 256'203'221;
  // std::size_t size = command == 't' ? 0 : 373'587'911;
  // std::size_t size = command == 't' ? 0 : 512'927'377;
  try
  {
    nnue_init("../net/nn-04cf2b4ed1da.nnue");
    std::locale::global(std::locale("en_US.UTF-8"));
    table table(size);
    auto time0 = std::chrono::high_resolution_clock::now();
    std::size_t count;
    int score;
    move move;
    switch (command) {
      case 'd':
        std::cout << "pre cum" << std::endl;
        count = position{fen}.divide(table, depth);
        break;
      case 'p':
        count = position{fen}.perft(table, depth);
        break;
      case 't':
        count = test::run("../epd/perft_long.txt", depth);
        break;
      case 'e':
        score = position{fen}.evaluate();
        count = 1;
        break;
      case 's':
        std::tie(score, move) = position{fen}.search(depth);
        count = counter;
        break;
      default:
        throw std::runtime_error("Unknown command");
    }
    auto time1 = std::chrono::high_resolution_clock::now();
    auto time = duration_cast<as_floating_point>(time1 - time0).count();
    std::cout << "s = " << score / 100.0 << std::endl;
    std::cout << "m = " << move << std::endl;
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
