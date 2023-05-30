#include "position.hpp"
#include "perft.hpp"
#include "search.hpp"
#include "test.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
  if (argc != 3 && argc != 4)
  {
    std::cerr << "Usage: " << argv[0] << " <command> <depth> [<fen>]" << std::endl;
    std::cerr << "Commands: " << "p[erft] | d[ivide] | t[est]" << std::endl;
    return 1;
  }

  char command = argv[1][0];
  std::size_t depth = std::atoll(argv[2]);
  std::string_view fen = argc == 4 ? argv[3] : "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  std::size_t size = 104'395'303;
  // std::size_t size = 1ull << 27;
  // 15'485'863;
  // 104'395'303;
  // 256'203'221;
  // 373'587'911;
  // 512'927'377;
  try
  {
    std::locale::global(std::locale("en_US.UTF-8"));
    switch (command) {
      case 'd': {
        perft_t<true> command{true, size};
        position{fen}.execute(command, depth);
      }
        break;
      case 'p': {
        perft_t<true> command{false, size};
        position{fen}.execute(command, depth);
      }
        break;
      case 't':
        test::run("../epd/perft_long.txt", depth);
        break;
      case 's': {
        nnue_init("../net/nn-04cf2b4ed1da.nnue");
        search_t command{size};
        position{fen}.execute(command, depth);
      }
        break;
      default:
        throw std::runtime_error("Unknown command");
    }
  }
  catch (const std::exception &exception)
  {
    std::cerr << exception.what() << std::endl;
  }
}
