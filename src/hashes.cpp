#include "hashes.hpp"
#include <random>
#include <algorithm>

static hash_t generate() {
    static std::mt19937_64 engine(123ull);
    static std::uniform_int_distribution<hash_t> distribution;
    return distribution(engine);
}

const hashes::lookup_t hashes::king_ = []() {
    lookup_t lookup;
    std::ranges::generate(lookup[WHITE], generate);
    std::ranges::generate(lookup[BLACK], generate);
    return lookup;
}();

const hashes::lookup_t hashes::queen_ = []() {
    lookup_t lookup;
    std::ranges::generate(lookup[WHITE], generate);
    std::ranges::generate(lookup[BLACK], generate);
    return lookup;
}();

const hashes::lookup_t hashes::rook_ = []() {
    lookup_t lookup;
    std::ranges::generate(lookup[WHITE], generate);
    std::ranges::generate(lookup[BLACK], generate);
    return lookup;
}();

const hashes::lookup_t hashes::bishop_ = []() {
    lookup_t lookup;
    std::ranges::generate(lookup[WHITE], generate);
    std::ranges::generate(lookup[BLACK], generate);
    return lookup;
}();

const hashes::lookup_t hashes::knight_ = []() {
    lookup_t lookup;
    std::ranges::generate(lookup[WHITE], generate);
    std::ranges::generate(lookup[BLACK], generate);
    return lookup;
}();

const hashes::lookup_t hashes::pawn_ = []() {
    lookup_t lookup;
    std::ranges::generate(lookup[WHITE], generate);
    std::ranges::generate(lookup[BLACK], generate);
    return lookup;
}();
