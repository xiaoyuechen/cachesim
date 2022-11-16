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

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <list>
#include <string>
#include <vector>

#define BLKSIZE 64

using Address = uint64_t;

Address BlkAddr(Address addr, size_t blksize) { return addr & ~(blksize - 1); }

size_t SetIdx(Address addr, size_t nset, size_t blksize) {
  return (addr & (nset * blksize * 8 - 1)) >> (int)(log2(blksize * 8));
}

struct Tag {
  bool valid = false;
  Address addr = 0;
};

struct Block {
  Tag tag{};

  bool OwnBlock(Address addr) const { return tag.valid && tag.addr == addr; }

  void Update(Address addr) {
    tag.valid = true;
    tag.addr = addr;
  }
};

struct Set {
  Set(size_t assoc) { blocks.resize(assoc); };

  bool Access(Address addr) {
    for (auto it = blocks.begin(); it != blocks.end(); ++it) {
      if (it->OwnBlock(addr)) {
        auto tmp = *it;
        blocks.erase(it);
        blocks.emplace_front(std::move(tmp));
        return true;
      }
    }

    /* no block found */
    auto blk = blocks.back();
    blocks.pop_back();
    blk.Update(addr);
    blocks.emplace_front(std::move(blk));
    return false;
  }

  std::list<Block> blocks;
};

struct Cache {
  Cache(size_t nset, size_t assoc, size_t nwarmup) {
    sets.resize(nset, Set(assoc));
    size = nset * assoc * BLKSIZE;
  }

  std::vector<Set> sets;
  size_t naccess = 0;
  size_t nhit = 0;
  size_t nwarmup = 0;
  size_t size = 0;

  void Access(Address addr) {
    auto idx = SetIdx(addr, sets.size(), BLKSIZE);
    auto hit = sets[idx].Access(BlkAddr(addr, BLKSIZE));

    if (!nwarmup) {
      ++naccess;
      nhit += hit;
    } else {
      --nwarmup;
    }
  }

  size_t Size() const { return size; }
};

int main(int argc, char *argv[]) {
  size_t nset = std::atoi(argv[1]);
  size_t assoc = std::atoi(argv[2]);
  const char *file = argv[3];

  auto cache = Cache(nset, assoc, 1'000'000);
  printf("Set: %zu, Assoc: %zu, Blk %zu, Size: %zu\n", nset, assoc,
         (size_t)BLKSIZE, cache.Size());

  auto fstream = std::ifstream();
  fstream.open(file);

  static constexpr size_t NCHAR = 64;
  char addr[NCHAR];

  while (!fstream.eof()) {
    fstream.getline(addr, NCHAR);
    cache.Access(std::atoll(addr));
  }

  printf("Access: %zu, Hit: %zu, Miss: %zu, MissRate: %f%%\n", cache.naccess,
         cache.nhit, cache.naccess - cache.nhit,
         ((float)(cache.naccess - cache.nhit)) / cache.naccess * 100);

  return 0;
}
