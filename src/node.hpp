#pragma once

#include "bitboard.hpp"
#include "bitboards.hpp"
#include "hash.hpp"
#include "hashes.hpp"
#include "move.hpp"
#include "side.hpp"
#include "nnue.hpp"

class node
{
    bitboard white;
    bitboard black;
    bitboard king_;
    bitboard rook_queen_;
    bitboard bishop_queen_;
    bitboard knight_;
    bitboard pawn_;
    bitboard castle;
    bitboard en_passant;
    hash_t hash_;
    node* parent_;
    const move* move_;

public:
    NNUEdata nnue;

    enum generation_t {all, captures};

    constexpr node() noexcept = default;

    constexpr node(bitboard white, bitboard black, bitboard king, bitboard rook_queen, bitboard bishop_queen, bitboard knight, bitboard pawn, bitboard castle, bitboard en_passant, hash_t hash) noexcept
        : white(white), black(black), king_(king), rook_queen_(rook_queen), bishop_queen_(bishop_queen), knight_(knight), pawn_(pawn), castle(castle), en_passant(en_passant), hash_(hash), parent_(nullptr), move_(nullptr) {
            nnue.accumulator.computedAccumulation = 0;
            nnue.dirtyPiece.dirtyNum = 0;
        }

    constexpr node(node& parent) noexcept
        : white(parent.white), black(parent.black), king_(parent.king_), rook_queen_(parent.rook_queen_), bishop_queen_(parent.bishop_queen_), knight_(parent.knight_), pawn_(parent.pawn_), castle(parent.castle), en_passant(0), hash_(parent.hash_), parent_(&parent), move_(nullptr) {
            nnue.accumulator.computedAccumulation = 0;
            nnue.dirtyPiece.dirtyNum = 0;
        }

    constexpr node* parent() const noexcept {
        return parent_;
    }

    constexpr const move* moved() const noexcept {
        return move_;
    }

    constexpr bitboard occupied() const noexcept {
        return white | black;
    }

    template <side_t side>
    constexpr bitboard occupied() const noexcept {
        return side == WHITE ? white : black;
    }

    template <side_t side>
    constexpr bitboard king() const noexcept {
        return king_ & occupied<side>();
    }

    template <side_t side>
    constexpr bitboard rook_queen() const noexcept {
        return rook_queen_ & occupied<side>();
    }

    template <side_t side>
    constexpr bitboard bishop_queen() const noexcept {
        return bishop_queen_ & occupied<side>();
    }

    constexpr bitboard queen() const noexcept {
        return rook_queen_ & bishop_queen_;
    }

    template <side_t side>
    constexpr bitboard queen() const noexcept {
        return queen() & occupied<side>();
    }

    template <side_t side>
    constexpr bitboard rook() const noexcept {
        return rook_queen_ & ~bishop_queen_ & occupied<side>();
    }

    template <side_t side>
    constexpr bitboard bishop() const noexcept {
        return bishop_queen_ & ~rook_queen_ & occupied<side>();
    }

    template <side_t side>
    constexpr bitboard knight() const noexcept {
        return knight_ & occupied<side>();
    }

    constexpr bitboard pawn() const noexcept {
        return pawn_;
    }

    template <side_t side>
    constexpr bitboard pawn() const noexcept {
        return pawn_ & occupied<side>();
    }

    template <side_t side>
    constexpr hash_t hash() const noexcept {
        return side == WHITE ? hash_ ^ castle ^ en_passant :  ~(hash_ ^ castle ^ en_passant);
    }

    template <side_t side>
    constexpr bitboard attackers() const noexcept;

    template <side_t side>
    bitboard attackers(square square) const noexcept;

    template <side_t side>
    bitboard checkers() const noexcept;

    template <side_t side, generation_t generation>
    std::span<move> generate(std::span<move, 256> moves) const noexcept;

    template <side_t side>
    void execute(const move& move) noexcept;

    // template <side_t side>
    std::pair<bitboard, const move*> execute(bitboard en_passant, const move* move) noexcept {
        auto e = this->en_passant;
        auto m = this->move_;
        this->en_passant = en_passant;
        this->move_ = move;
        // nnue.accumulator.computedAccumulation = 0;
        return {e, m};
    }

    template <side_t side>
    int evaluate() noexcept;
};

template <side_t side>
constexpr bitboard node::attackers() const noexcept {
  bitboard out = 0ull;
  out |= bitboards::king(king<side>());
  out |= bitboards::knight(knight<side>());
  out |= bitboards::slider(rook_queen<side>(), bishop_queen<side>(), occupied() & ~king<~side>());
  out |= bitboards::pawn<side>(pawn<side>());
  return out;
}

template <side_t side>
inline bitboard node::attackers(square square) const noexcept {
  bitboard out = 0ull;
  out |= bitboards::king(square) & king<side>();
  out |= bitboards::knight(square) & knight<side>();
  out |= bitboards::rook_queen(square, occupied()) & rook_queen<side>();
  out |= bitboards::bishop_queen(square, occupied()) & bishop_queen<side>();
  out |= bitboards::pawn<~side>(square) & pawn<side>();
  return out;
}

template <side_t side>
inline bitboard node::checkers() const noexcept {
  auto square = king<side>().find();
  return attackers<~side>(square);
}
