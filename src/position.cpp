#include "position.hpp"
#include <regex>
#include <iostream>

position::position() :
  root_{
    "12"_r,
    "78"_r,
    "e1e8"_b,
    "a1d1h1a8d8h8"_b,
    "c1d1f1c8d8f8"_b,
    "b1g1b8g8"_b,
    "27"_r,
    "a1h1a8h8"_b,
    {}},
  side_{WHITE}
{}

static const std::regex fen_regex("(.*)/(.*)/(.*)/(.*)/(.*)/(.*)/(.*)/(.*) ([wb]) ([-KQkq]+) ([-a-h1-8]+)( \\d+)?( \\d+)?");

position::position(std::string_view fen)
{
  std::cmatch match;
  if (!std::regex_search(fen.data(), match, fen_regex))
    throw std::runtime_error("fen not matched by regex");
  bitboard white;
  bitboard black;
  bitboard king;
  bitboard rook_queen;
  bitboard bishop_queen;
  bitboard knight;
  bitboard pawn;
  bitboard castle;
  bitboard en_passant;
  for (int i = 0; i < 8; ++i) {
    int j = 0;
    for (auto ch : match[8 - i].str()) {
      if (std::isdigit(ch))
        j += ch - '0';
      else {
        uint64_t b = 1ull << (8 * i + j);
        switch (std::tolower(ch)) {
        case 'r':
          rook_queen |= b;
          break;
        case 'b':
          bishop_queen |= b;
          break;
        case 'q':
          rook_queen |= b;
          bishop_queen |= b;
          break;
        case 'k':
          king |= b;
          break;
        case 'p':
          pawn |= b;
          break;
        case 'n':
          knight |= b;
          break;
        }
        if (std::isupper(ch))
          white |= b;
        else
          black |= b;
        ++j;
      }
    }
  }
  side_ = match[9].str()[0] == 'w' ? WHITE : BLACK;
  if (match[10].compare("-")) {
    for (auto ch : match[10].str()) {
      switch (ch) {
      case 'K':
        castle |= "h1"_b;
        break;
      case 'Q':
        castle |= "a1"_b;
        break;
      case 'k':
        castle |= "h8"_b;
        break;
      case 'q':
        castle |= "a8"_b;
        break;
      }
    }
  }
  if (match[11].compare("-")) {
    en_passant = bitboard{match[11].str()};
  }
  root_ = {white, black, king, rook_queen, bishop_queen, knight, pawn, castle, en_passant};
}

template <side_t side>
std::tuple<std::size_t, std::size_t> position::perft(const node &current, int depth) noexcept
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

std::tuple<std::size_t, std::size_t> position::perft(int depth) const noexcept {
  return side_ == WHITE ? perft<WHITE>(root_, depth) : perft<BLACK>(root_, depth);
}

template <side_t side>
std::size_t position::perft_bulk(const node &current, int depth) noexcept
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

std::size_t position::perft_bulk(int depth) const noexcept {
  return side_ == WHITE ? perft_bulk<WHITE>(root_, depth) : perft_bulk<BLACK>(root_, depth);
}

template <side_t side>
std::size_t position::perft_divide(const node &current, int depth) noexcept
{
  std::size_t count = 0;
  std::array<move, 256> buffer;
  auto moves = current.generate<side>(buffer);
  for (const auto &move : moves)
  {
    std::size_t count_;
    if (depth <= 1)
      count_ = 1;
    else
    {
      node succ(current);
      succ.execute<side>(move);
      std::array<struct move, 256> buffer2;
      count_ = depth == 2 ? succ.generate<~side>(buffer2).size() : perft_bulk<~side>(succ, depth - 1);
    }
    count += count_;
    std::cout << move << " " << count_ << std::endl;
  }
  return count;
}

std::size_t position::perft_divide(int depth) const noexcept {
  return side_ == WHITE ? perft_divide<WHITE>(root_, depth) : perft_divide<BLACK>(root_, depth);
}
