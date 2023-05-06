#include "test.hpp"
#include <atomic>
#include <regex>
#include <thread>
#include <semaphore>
#include <fstream>
#include <iostream>
#include <format>

static const std::regex re2("(.*),(.*),(.*),(.*),(.*),(.*),(.*)");

test::test(std::string_view row, std::size_t index) {
  std::cmatch m;
  if (!std::regex_search(row.data(), m, re2))
    throw std::runtime_error("broken row");
  position_ = {m[1].str()};
  for (int i = 0; i < 6; ++i) {
    expected_[i] = std::atoll(m[2 + i].str().data());
  }
  index_ = index;
}

std::expected<size_t, std::string> test::run(int depth) const noexcept {
  size_t count = position_.perft_bulk(depth);
  if (count != expected_[depth - 1])
    return std::unexpected(std::format("kaputtnik: l={}, d={}, n={}, e={}", index_, depth, count, expected_[depth - 1]));
  return count;
}

size_t test::run(std::string_view file, int depth) {
  std::atomic_size_t count{0};
  std::vector<std::jthread> workers{};
  std::counting_semaphore<> slots{std::jthread::hardware_concurrency()};
  std::ifstream in{file.data()};
  std::size_t index = 0;
  while (!in.eof()) {
    ++index;
    slots.acquire();
    char line[256];
    in.getline(line, 256);
    // std::cout << line << std::endl;
    const struct test test{line, index};
    workers.emplace_back([test, depth, &count, &slots]() noexcept {
      const auto result = test.run(depth);
      if (result.has_value()) {
        count += result.value();
      } else {
        std::cerr << result.error() << std::endl;
      }
      slots.release();
    });
  }
  return count;
}
