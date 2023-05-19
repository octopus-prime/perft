#include "position.hpp"
#include "table.hpp"
#include "history.hpp"
#include "transposition.hpp"
#include <algorithm>
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
  auto moves = current.generate<side, node::all>(buffer);
  for (const auto &move : moves)
  {
    if (depth <= 1)
      count += 1;
    else
    {
      node succ((node &) current);
      succ.execute<side>(move);
      std::array<struct move, 256> buffer2;
      count += depth == 2 ? succ.generate<~side, node::all>(buffer2).size() : perft<~side>(succ, depth - 1);
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
  auto moves = current.generate<side, node::all>(buffer);
  for (const auto &move : moves)
  {
    if (depth <= 1)
      count += 1;
    else
    {
      node succ((node &) current);
      succ.execute<side>(move);
      std::array<struct move, 256> buffer2;
      count += depth == 2 ? succ.generate<~side, node::all>(buffer2).size() : perft<~side>(succ, table, depth - 1);
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
  auto moves = current.generate<side, node::all>(buffer);
  for (const auto &move : moves)
  {
    std::size_t count_;
    if (depth <= 1)
      count_ = 1;
    else
    {
      node succ((node &) current);
      succ.execute<side>(move);
      std::array<struct move, 256> buffer2;
      count_ = depth == 2 ? succ.generate<~side, node::all>(buffer2).size() : perft<~side>(succ, depth - 1);
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
  auto moves = current.generate<side, node::all>(buffer);
  for (const auto &move : moves)
  {
    std::size_t count_;
    if (depth <= 1)
      count_ = 1;
    else
    {
      node succ((node &) current);
      succ.execute<side>(move);
      std::array<struct move, 256> buffer2;
      count_ = depth == 2 ? succ.generate<~side, node::all>(buffer2).size() : perft<~side>(succ, table, depth - 1);
    }
    count += count_;
    std::cout << move << '\t' << count_ << std::endl;
  }
  return count;
}

std::size_t position::divide(table& table, int depth) const noexcept {
  return side_ == WHITE ? divide<WHITE>(root_, table, depth) : divide<BLACK>(root_, table, depth);
}

int position::evaluate() noexcept {
  return side_ == WHITE ? root_.evaluate<WHITE>() : -root_.evaluate<BLACK>();
}

std::size_t counter;
history_t history;
transposition_t transposition(104'395'303);

template <side_t side>
uint32_t score(node const& current, const move& move) noexcept {
  if (!(current.occupied<~side>() & bitboard{move.to()}))
    return 0;

  uint32_t score = 0;
  if (current.queen<~side>() & bitboard{move.to()})
    score += 900;
  if (current.rook<~side>() & bitboard{move.to()})
    score += 500;
  if (current.bishop<~side>() & bitboard{move.to()})
    score += 350;
  if (current.knight<~side>() & bitboard{move.to()})
    score += 300;
  if (current.pawn<~side>() & bitboard{move.to()})
    score += 100;
  switch (move.type()) {
    case move::KING:
      score -= 100;
      break;
    case move::QUEEN:
      score -= 90;
      break;
    case move::ROOK:
      score -= 50;
      break;
    case move::BISHOP:
      score -= 35;
      break;
    case move::KNIGHT:
      score -= 30;
      break;
    case move::PAWN:
      score -= 10;
      break;
    case move::PROMOTE_QUEEN:
      score += 900 - 10;
      break;
    case move::PROMOTE_ROOK:
      score += 500 - 10;
      break;
    case move::PROMOTE_BISHOP:
      score += 350 - 10;
      break;
    case move::PROMOTE_KNIGHT:
      score += 300 - 10;
      break;
    case move::EN_PASSANT:
      score += 100 - 10;
      break;
  }

  return score;
}

template <side_t side>
int position::search(node& current, int alpha, int beta) noexcept {
    // ++counter;
	int eval = current.evaluate<side>();

	if (eval >= beta)
		return eval;
	if (eval > alpha)
		alpha = eval;

  std::array<move, 256> buffer;
	auto moves = current.generate<side, node::captures>(buffer);

  std::array<uint32_t, 256> scores;
  std::ranges::transform(moves, scores.begin(), [&current](const move& move) noexcept { return score<side>(current, move); });
  std::ranges::sort(std::views::zip(moves, scores), [](auto&& lhs, auto&& rhs) noexcept { return std::get<1>(lhs) > std::get<1>(rhs); });

	for (move const& move : moves) {
    node succ(current);
    succ.execute<side>(move);
		int score = -search<~side>(succ, -beta, -alpha);
		if (score >= beta)
			return score;
		if (score > alpha)
			alpha = score;
	}

	return alpha;
}


template <side_t side>
std::pair<int, move> position::search(node &current, int alpha, int beta, int depth) noexcept {
  ++counter;

  if (depth == 0) {
    int score = search<side>(current, alpha, beta);
    return {score, {}};
  }

  std::array<move, 256> buffer;
  auto moves = current.generate<side, node::all>(buffer);
  if (moves.empty())
    return {current.checkers<side>() ? -32000 : 0, {}};

  move best;

	const entry_t* const entry = transposition.get<side>(current);
	if (entry)
	{
		if (entry->depth >= depth)
		{
			switch (entry->flag)
			{
			case flag_t::EXACT:
				return {entry->score, entry->move};
			case flag_t::LOWER:
				if (entry->score > alpha)
					alpha = entry->score;
				break;
			case flag_t::UPPER:
				if (entry->score < beta)
					beta = entry->score;
				break;
			default:
				break;
			}
			if (alpha >= beta)
				return {alpha, entry->move};
		}
		best = entry->move;
	}

	// if (best == move{} && depth > 2)
	// 	best = std::get<1>(search<side>(current, alpha, beta, depth - 2));

  std::array<uint32_t, 256> scores;
  std::ranges::transform(moves, scores.begin(), [&current, &best](const move& move) noexcept {
    if (move == best)
      return std::numeric_limits<uint32_t>::max();

    uint32_t score_ = score<side>(current, move) * 100'000;
    return score_ + (uint32_t) history.get<side>(move);
  });
  std::ranges::sort(std::views::zip(moves, scores), [](auto&& lhs, auto&& rhs) noexcept { return std::get<1>(lhs) > std::get<1>(rhs); });

  bool pv = false;
  int index = 0;
  for (const auto &move : moves) {
    ++index;
    int reduction = 0;
    history.put_all<side>(move, depth);
    node succ(current);
    succ.execute<side>(move);
    int score;
    if (!pv)
      score = -std::get<0>(search<~side>(succ, -beta, -alpha, depth - 1));
    else {
      // if (depth > 3 && index > 6)//moves.size() / 3)
      //   reduction = depth / 3;
      score = -std::get<0>(search<~side>(succ, -alpha - 1, -alpha, depth - 1 - reduction));
      if (score > alpha)
        score = -std::get<0>(search<~side>(succ, -beta, -alpha, depth - 1));
    }
    if (score >= beta) {
	    transposition.put<side>(current, move, beta, LOWER, depth);
      history.put_good<side>(move, depth);
      return {beta, move};
    }
    if (score > alpha) {
      alpha = score;
      best = move;
      pv = true;
    }
  }

  if (pv) {
    history.put_good<side>(best, depth);
	  transposition.put<side>(current, best, alpha, EXACT, depth);
  } else {
	  transposition.put<side>(current, best, alpha, UPPER, depth);
  }

  return {alpha, best};
}

std::pair<int, move> operator-(const std::pair<int, move>& pair) noexcept {
  return {-pair.first, pair.second};
}

std::pair<int, move> position::search(int depth) noexcept {
  counter = 0;
  int alpha = -32000;
  int beta  = +32000;
  int score;
  move move;
  for (int i = 1; i <= depth; ++i) {
    std::tie(score, move) = side_ == WHITE ? search<WHITE>(root_, alpha, beta, i) : -search<BLACK>(root_, alpha, beta, i);
    std::cout << "i = " << i << std::endl;
    std::cout << "s = " << score / 100.0 << std::endl;
    std::cout << "m = " << move << std::endl;
    std::cout << std::endl;
  }
  return {score, move};
}
