#include "position.hpp"
#include <algorithm>
#include <regex>
#include <format>
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
        auto s = 8 * i + j;
        uint64_t b = 1ull << s;
        switch (ch) {
        case 'K':
          king |= b;
          white |= b;
          hash ^= hashes::king<WHITE>(s);
          break;
        case 'k':
          king |= b;
          black |= b;
          hash ^= hashes::king<BLACK>(s);
          break;
        case 'Q':
          rook_queen |= b;
          bishop_queen |= b;
          white |= b;
          hash ^= hashes::queen<WHITE>(s);
          break;
        case 'q':
          rook_queen |= b;
          bishop_queen |= b;
          black |= b;
          hash ^= hashes::queen<BLACK>(s);
          break;
        case 'R':
          rook_queen |= b;
          white |= b;
          hash ^= hashes::rook<WHITE>(s);
          break;
        case 'r':
          rook_queen |= b;
          black |= b;
          hash ^= hashes::rook<BLACK>(s);
          break;
        case 'B':
          bishop_queen |= b;
          white |= b;
          hash ^= hashes::bishop<WHITE>(s);
          break;
        case 'b':
          bishop_queen |= b;
          black |= b;
          hash ^= hashes::bishop<BLACK>(s);
          break;
        case 'N':
          knight |= b;
          white |= b;
          hash ^= hashes::knight<WHITE>(s);
          break;
        case 'n':
          knight |= b;
          black |= b;
          hash ^= hashes::knight<BLACK>(s);
          break;
        case 'P':
          pawn |= b;
          white |= b;
          hash ^= hashes::pawn<WHITE>(s);
          break;
        case 'p':
          pawn |= b;
          black |= b;
          hash ^= hashes::pawn<BLACK>(s);
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
