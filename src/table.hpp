#pragma once

#include "hash.hpp"
#include <vector>
#include <optional>

class table {
    struct entry {
        hash_t hash = -1;
        std::size_t count = 0;
        int depth = -1;
    };

    std::vector<entry> entries_;

public:
    constexpr table() : entries_() { /*clear();*/ }
    table(std::size_t size) : entries_(size) { /*clear();*/ }

    // void clear() noexcept {
    //     for (entry& e : entries_) {
    //         e.hash = -1;
    //         e.count = 0;
    //         e.depth = 0;
    //     }
    // }

    constexpr bool empty() const noexcept {
        return entries_.empty();
    }

    void put(hash_t hash, int depth, std::size_t count) noexcept {
        entry& e = entries_[hash % entries_.size()];
        if (e.depth <= depth && e.count < count) {
            e.hash = hash;
            e.depth = depth;
            e.count = count;
        }
    }

    std::optional<std::size_t> get(hash_t hash, int depth) const noexcept {
        const entry& e = entries_[hash % entries_.size()];
        return e.hash == hash && e.depth == depth ? std::make_optional(e.count) : std::nullopt;
    }
};
