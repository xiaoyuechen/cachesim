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

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <list>
#include <vector>

using Address = uint64_t;

constexpr Address BlkAddr(Address addr, size_t blksize) {
  return addr & ~(blksize - 1);
}

constexpr size_t SetIdx(Address addr, size_t nset, size_t blksize) {
  return (addr & (nset * blksize * 8 - 1)) >> (std::countr_zero(blksize) + 3);
}

struct Tag {
  bool valid = false;
  Address addr = 0;
};

struct Block {
  bool OwnBlock(Address addr) const { return tag.valid && tag.addr == addr; }

  void Update(Address addr) {
    tag.valid = true;
    tag.addr = addr;
  }

  Tag tag = {};
};

struct Set {
  Set(size_t assoc) { blocks.resize(assoc); };

  bool Access(Address addr) {
    auto ownblk =
        std::find_if(blocks.begin(), blocks.end(),
                     [addr](const auto &blk) { return blk.OwnBlock(addr); });

    if (ownblk != blocks.end()) {
      blocks.splice(blocks.begin(), blocks, ownblk);
      return true;
    } else {
      blocks.splice(blocks.begin(), blocks, --blocks.end());
      blocks.front().Update(addr);
      return false;
    }
  }

  std::list<Block> blocks;
};

struct Cache {
  Cache(size_t nset, size_t assoc, size_t blksize) {
    this->nset = nset;
    this->assoc = assoc;
    this->blksize = blksize;
    sets.resize(nset, Set(assoc));
  }

  bool Access(Address addr) {
    auto idx = SetIdx(addr, sets.size(), blksize);
    return sets[idx].Access(BlkAddr(addr, blksize));
  }

  size_t Size() const { return nset * assoc * blksize; }

  std::vector<Set> sets;
  size_t nset = 0;
  size_t assoc = 0;
  size_t blksize = 0;
};

struct Arg {
  size_t nset;
  size_t assoc;
  size_t blksize;
};

int main(int argc, char *argv[]) {
  union {
    Arg arg = {.nset = 16, .assoc = 16, .blksize = 64};
    size_t dummy[3];
  };

  std::transform(argv + 1, argv + argc, dummy, std::atoll);

  auto cache = Cache(arg.nset, arg.assoc, arg.blksize);
  printf("Set: %zu, Assoc: %zu, Blk %zu, Size: %zu\n", arg.nset, arg.assoc,
         arg.blksize, cache.Size());

  static constexpr size_t NCHAR = 64;
  char addr[NCHAR];

  size_t naccess = 0;
  size_t nhit = 0;

  while (!std::cin.eof()) {
    std::cin.getline(addr, NCHAR);
    ++naccess;
    nhit += cache.Access(std::atoll(addr));
  }

  printf("Access: %zu, Hit: %zu, Miss: %zu, MissRate: %f%%\n", naccess, nhit,
         naccess - nhit, ((float)(naccess - nhit)) / naccess * 100);

  return 0;
}
