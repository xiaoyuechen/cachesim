/*
 * Copyright (C) 2022  Xiaoyue Chen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <stddef.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using Address = uint64_t;

struct Arg {
  size_t n_cache_line = 16 * 16;
  size_t window_size = 200'000;
  float sample_rate = 1e-4;
  float threshold = 1e-4;
};

constexpr Address BlkAddr(Address addr, size_t blksize) {
  return addr & ~(blksize - 1);
}

int main(int argc, char *argv[]) {
  size_t n_cache_line = 16 * std::atoi(argv[1]);
  size_t blksize = 64;
  size_t window_size = 10'000'000;
  //float sample_rate = 1e-4;
  float sample_rate = 1e-2;
  float threshold = 1e-4;

  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(1, 1 / sample_rate);

  std::unordered_map<Address, size_t> watch;
  std::unordered_map<size_t, size_t> hist;
  std::vector<float> window_r;

  while (!std::cin.eof()) {
    size_t win_i;
    for (win_i = 0; win_i < window_size; ++win_i) {
      if (std::cin.eof())
        break;

      static constexpr size_t NCHAR = 64;
      char buff[NCHAR];
      std::cin.getline(buff, NCHAR);
      Address addr = BlkAddr(std::atoll(buff), blksize);

      auto found = watch.find(addr);
      if (found != watch.end()) {
        size_t distance = win_i - found->second;
        ++hist[distance];
        watch.erase(found);
      }

      int roll = distribution(generator);
      if (roll == 1) {
        watch[addr] = win_i;
      }
    }

    /* Finish sampling one window */
    float r = 0.5f;
    while (true) {
      float lhs = r * sample_rate * win_i;
      float rhs =
          std::accumulate(
              hist.begin(), hist.end(), 0.0f,
              [n_cache_line, r](float acc, auto pair) {
                auto [dist, count] = pair;
                return acc + count * (1.0f - pow(1.0f - 1.0f / n_cache_line,
                                                 r * dist));
              }) + watch.size(); /* the cold misses */

      if (std::abs(rhs - lhs) / (sample_rate * win_i) < threshold) {
        window_r.push_back(r);
//        printf("window %zu: %f%%\n", window_r.size(), window_r.back() * 100);
        break;
      }

      r += r / 2 * (lhs > rhs ? -1 : 1);
    }
    watch.clear();
    hist.clear();
  }

  float sum =
      std::accumulate(window_r.begin(), window_r.end(), 0.0f) / window_r.size();

  printf("Average: %f%%\n", sum * 100);

  return 0;
}
