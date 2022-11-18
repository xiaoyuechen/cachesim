# Copyright (C) 2022  Xiaoyue Chen
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

CXXFLAGS = -O3 -std=c++20 -fno-exceptions

.PHONY: all
all: ccache micro-benchmark

ccache:

micro-benchmark: micro-benchmark.c
	$(CC) -O0 -g micro-benchmark.c -o micro-benchmark

.PHONY: clean
clean:
	rm -f ccache micro-benchmark

.PHONY: experiment
experiment: ccache
	./experiment
