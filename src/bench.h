/* bench.h - Redpile benchmarks and stress tests
 *
 * Copyright (C) 2014 Ryan Mendivil <ryan@nullreff.net>
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

#ifndef REDPILE_BENCH_H
#define REDPILE_BENCH_H

#define RANGE(var,start,end) Coord var; for (var = start; var <= end; var++) {

#define RANGE_END }

#define CUBE_RANGE(start,end)\
    RANGE(x,start,end)\
    RANGE(y,start,end)\
    RANGE(z,start,end)

#define CUBE_RANGE_END RANGE_END RANGE_END RANGE_END

#define BENCHMARK_START(name) do {\
    char* message = #name;\
    long long start_time = get_time();

#define BENCHMARK_END\
    print_time(message, get_time() - start_time);\
} while(0);

void run_benchmarks(void);

#endif

