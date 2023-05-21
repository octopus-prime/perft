#include "test.hpp"
#include <atomic>
#include <regex>
#include <thread>
#include <fstream>
#include <iostream>
#include <format>

static const std::regex epd_regex("(.*),(.*),(.*),(.*),(.*),(.*),(.*)");

test::test(std::string_view epd, std::size_t index) {
  std::cmatch match;
  if (!std::regex_search(epd.data(), match, epd_regex))
    throw std::runtime_error("broken epd");
  position_ = {match[1].str()};
  for (int i = 0; i < 6; ++i)
    expected_[i] = std::atoll(match[2 + i].str().data());
  index_ = index;
}

std::expected<size_t, std::string> test::run(int depth) const noexcept {
  size_t count = position_.perft(depth);
  if (count != expected_[depth - 1])
    return std::unexpected(std::format("kaputtnik: l={}, d={}, n={}, e={}", index_, depth, count, expected_[depth - 1]));
  return count;
}

class blocking_input {
  std::ifstream stream;
  std::size_t index;
  std::mutex mutex;

public:
  blocking_input(std::string_view file) : stream{file.data()}, index{}, mutex{} {}

  std::size_t read(std::span<char, 256> epd) {
    std::lock_guard<std::mutex> lock(mutex);
    if (!stream.good())
      return 0;
    stream.getline(epd.data(), epd.size());
    return ++index;
  }
};

size_t test::run(std::string_view file, int depth) {
  blocking_input input{file};
  std::atomic_size_t count{0};
  {
    std::vector<std::jthread> workers{};
    for (int i = 0; i < std::jthread::hardware_concurrency(); ++i) {
      workers.emplace_back([depth, &input, &count]() noexcept {
        char epd[256];
        while (auto index = input.read(epd) != 0) {
          // std::cout << epd << std::endl;
          const auto result = test{epd, index}.run(depth);
          if (result.has_value())
            count += result.value();
          else
            std::cerr << result.error() << std::endl;
        }
      });
    }
  }
  return count;
}
