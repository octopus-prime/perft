#include "search.hpp"
#include <chrono>
#include <format>
#include <iostream>

template <side_t side>
uint32_t score(node const& current, const move& move) noexcept {
  if (!(current.occupied<~side>() & bitboard{move.to()}) && move.type() < move::PROMOTE_QUEEN)
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
      score -= 10;
      break;
    case move::QUEEN:
      score -= 9;
      break;
    case move::ROOK:
      score -= 5;
      break;
    case move::BISHOP:
      score -= 4;
      break;
    case move::KNIGHT:
      score -= 3;
      break;
    case move::PAWN:
      score -= 1;
      break;
    case move::PROMOTE_QUEEN:
      score += 900 - 100;
      break;
    case move::PROMOTE_ROOK:
      score += 500 - 100;
      break;
    case move::PROMOTE_BISHOP:
      score += 350 - 100;
      break;
    case move::PROMOTE_KNIGHT:
      score += 300 - 100;
      break;
    case move::EN_PASSANT:
      score += 100;
      break;
  }

  return score;
}

template <side_t side>
int search_t::search(node& current, int alpha, int beta) noexcept {
    // ++counter;
	int eval = current.evaluate<side>();

	if (eval >= beta)
		return beta;
  int delta = 1200;
  constexpr bitboard rank = side == WHITE ? "7"_r : "2"_r;
  if (current.pawn<side>() & rank)
    delta += 800;
	if (eval < alpha - delta)
		return alpha;
	if (eval > alpha)
		alpha = eval;

  std::array<move, 256> buffer;
	auto moves = current.generate<side, node::captures>(buffer);

  std::array<uint32_t, 256> scores;
  std::ranges::transform(moves, scores.begin(), [&current](const move& move) noexcept { return score<side>(current, move); });
  std::ranges::sort(std::views::zip(moves, scores), std::greater{}, [](auto&& tuple) noexcept { return std::get<1>(tuple); });

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
inline bool
try_null(const node& node) noexcept {
	return node.occupied<side>().count() > 3
		&& (node.knight<side>() | node.bishop_queen<side>() | node.rook_queen<side>());
//		&& !(node.attack<other_tag>() & detail::attacker<king_tag, color_tag>::attack(node.occupy<king_tag, color_tag>()));
}

template <side_t side>
std::pair<int, move> search_t::search(node &current, int alpha, int beta, int depth) noexcept {
  ++counter;

  bool check = current.checkers<side>();
  depth += check;

  if (depth == 0) {
    int score = search<side>(current, alpha, beta);
    return {score, {}};
  }

  std::array<move, 256> buffer;
  auto moves = current.generate<side, node::all>(buffer);
  if (moves.empty())
    return {check ? -32000 : 0, {}};

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
				return {beta, entry->move};
		}
		best = entry->move;
	}

	const std::uint_fast8_t reduction = 2 + (depth > 6);
	const bool skip = entry && entry->depth > depth - reduction && entry->score < beta && entry->flag == flag_t::UPPER;
	if (depth > reduction  && current.moved() != nullptr  && !check && try_null<side>(current) && !skip) {
    // node succ(current);
    auto [e, m] = current.execute(0, nullptr);
		int score = -std::get<0>(search<~side>(current, -beta, -beta + 1, depth - reduction));
    current.execute(e, m);
    if (score >= beta) {
		  score = std::get<0>(search<side>(current, beta - 1, beta, depth - reduction));
      if (score >= beta)
  			return { beta, {} };
    }
	}

	if (best == move{} && depth > 2)
		best = std::get<1>(search<side>(current, alpha, beta, depth - 2));

  std::array<uint32_t, 256> scores;
  std::ranges::transform(moves, scores.begin(), [this, &current, &best](const move& move) noexcept {
    if (move == best)
      return std::numeric_limits<uint32_t>::max();

    uint32_t score_ = score<side>(current, move) * 100'000;
    return score_ + (uint32_t) history.get<side>(move);
  });
  std::ranges::sort(std::views::zip(moves, scores), std::greater{}, [](auto&& tuple) noexcept { return std::get<1>(tuple); });

  bool pv = false;
  int index = 0;
  for (const auto &move : moves) {
    ++index;
    int reduction = 0;
    history.put_all<side>(move, depth);
    node succ(current);
    succ.execute<side>(move);
    int score;
    bool capture = current.occupied<~side>() & bitboard{move.to()};
    if (!pv || check || succ.checkers<~side>() || capture)
      score = -std::get<0>(search<~side>(succ, -beta, -alpha, depth - 1));
    else {
      if (/*depth > 3 &&*/ index > moves.size() / 2)
        reduction += depth / 3;
      score = -std::get<0>(search<~side>(succ, -alpha - 1, -alpha, depth - 1 - reduction));
      if (score > alpha)// && score < beta)
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

void search_t::execute(node& current, side_t side, int depth)  {
  using as_floating_point = std::chrono::duration<double, std::ratio<1>>;
  constexpr int alpha = -32000;
  constexpr int beta  = +32000;
  auto time0 = std::chrono::high_resolution_clock::now();
  for (int i = 1; i <= depth; ++i) {
    auto [score, move] = side == WHITE ? search<WHITE>(current, alpha, beta, i) : -search<BLACK>(current, alpha, beta, i);
    auto time1 = std::chrono::high_resolution_clock::now();
    auto time = duration_cast<as_floating_point>(time1 - time0).count();
    std::cout << std::format("{:2d} {:7.3f} {:12L} {:10L} {:6.2f} ", i , time, counter, size_t(counter / time), score / 100.0) << move << std::endl;
  }
}
