#include "position.hpp"
#include "table.hpp"
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
    0,
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
  hash_t hash;
  for (int i = 0; i < 8; ++i) {
    int j = 0;
    for (auto ch : match[8 - i].str()) {
      if (std::isdigit(ch))
        j += ch - '0';
      else {
        uint64_t b = 1ull << (8 * i + j);
        switch (ch) {
        case 'K':
          king |= b;
          white |= b;
          hash ^= hashes::king<WHITE>(b);
          break;
        case 'k':
          king |= b;
          black |= b;
          hash ^= hashes::king<BLACK>(b);
          break;
        case 'Q':
          rook_queen |= b;
          bishop_queen |= b;
          white |= b;
          hash ^= hashes::queen<WHITE>(b);
          break;
        case 'q':
          rook_queen |= b;
          bishop_queen |= b;
          black |= b;
          hash ^= hashes::queen<BLACK>(b);
          break;
        case 'R':
          rook_queen |= b;
          white |= b;
          hash ^= hashes::rook<WHITE>(b);
          break;
        case 'r':
          rook_queen |= b;
          black |= b;
          hash ^= hashes::rook<BLACK>(b);
          break;
        case 'B':
          bishop_queen |= b;
          white |= b;
          hash ^= hashes::bishop<WHITE>(b);
          break;
        case 'b':
          bishop_queen |= b;
          black |= b;
          hash ^= hashes::bishop<BLACK>(b);
          break;
        case 'N':
          knight |= b;
          white |= b;
          hash ^= hashes::knight<WHITE>(b);
          break;
        case 'n':
          knight |= b;
          black |= b;
          hash ^= hashes::knight<BLACK>(b);
          break;
        case 'P':
          pawn |= b;
          white |= b;
          hash ^= hashes::pawn<WHITE>(b);
          break;
        case 'p':
          pawn |= b;
          black |= b;
          hash ^= hashes::pawn<BLACK>(b);
          break;
        }
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
  root_ = {white, black, king, rook_queen, bishop_queen, knight, pawn, castle, en_passant, hash};
}

template <side_t side>
std::size_t position::perft(const node &current, int depth) noexcept
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
      count += depth == 2 ? succ.generate<~side>(buffer2).size() : perft<~side>(succ, depth - 1);
    }
  }
  return count;
}

std::size_t position::perft(int depth) const noexcept {
  return side_ == WHITE ? perft<WHITE>(root_, depth) : perft<BLACK>(root_, depth);
}

template <side_t side>
std::size_t position::perft(const node &current, table& table, int depth) noexcept
{
  auto hit = table.get(current.hash<side>(), depth);
  if (hit.has_value()) {
    return hit.value();
  }
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
      count += depth == 2 ? succ.generate<~side>(buffer2).size() : perft<~side>(succ, table, depth - 1);
    }
  }
  table.put(current.hash<side>(), depth, count);
  return count;
}

std::size_t position::perft(table& table, int depth) const noexcept {
  return side_ == WHITE ? perft<WHITE>(root_, table, depth) : perft<BLACK>(root_, table, depth);
}

template <side_t side>
std::size_t position::divide(const node &current, int depth) noexcept
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
      count_ = depth == 2 ? succ.generate<~side>(buffer2).size() : perft<~side>(succ, depth - 1);
    }
    count += count_;
    std::cout << move << '\t' << count_ << std::endl;
  }
  return count;
}

std::size_t position::divide(int depth) const noexcept {
  return side_ == WHITE ? divide<WHITE>(root_, depth) : divide<BLACK>(root_, depth);
}

template <side_t side>
std::size_t position::divide(const node &current, table& table, int depth) noexcept
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
      count_ = depth == 2 ? succ.generate<~side>(buffer2).size() : perft<~side>(succ, table, depth - 1);
    }
    count += count_;
    std::cout << move << '\t' << count_ << std::endl;
  }
  return count;
}

std::size_t position::divide(table& table, int depth) const noexcept {
  return side_ == WHITE ? divide<WHITE>(root_, table, depth) : divide<BLACK>(root_, table, depth);
}
