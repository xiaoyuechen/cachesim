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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  uint8_t dummy[64];
} Blk;

typedef struct {
  Blk blks[2048];
} DataSet;

DataSet datasets[128];

int main() {
  for (size_t i = 0; i < 128; ++i) {
    for (size_t j = 0; j < 2; ++j) {
      for (size_t k = 0; k < 2048; ++k) {
        uint8_t tmp = datasets[i].blks[k].dummy[0];
        uint8_t tmp1 = datasets[i].blks[k].dummy[0];
      }
    }
  }

  return 0;
}
