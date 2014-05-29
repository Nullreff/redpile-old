/* bench.c - Redpile benchmarks and stress tests
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

#include "bench.h"
#include "block.h"
#include "redstone.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define TO_SECONDS(x) ((x) / (1000 * 1000))
#define MILISECONDS(x) ((x) * 1000)
#define BENCHMARK(METHOD,PER_ITER,LIMIT)\
    do {\
        long long start, time, count;\
        count = 0;\
        start = get_time();\
        do\
        {\
            for (int i = 0; i < PER_ITER; i++)\
            {\
                benchmark_ ## METHOD(world);\
            }\
            count += PER_ITER;\
            time = get_time() - start;\
        } while (time < LIMIT);\
        float final = count / TO_SECONDS((float)time);\
        if (final > 1000)\
        {\
            final /= 1000;\
            printf(#METHOD ": %.2fk / sec\n", final);\
        }\
        else\
        {\
            printf(#METHOD ": %.2f / sec\n", final);\
        }\
    } while (0) 

static long long get_time(void)
{
    struct timeval value;
    gettimeofday(&value, NULL);

    long long time = ((long long)value.tv_sec) * 1000000;
    time += value.tv_usec;
    return time;
}

static void benchmark_insert(World* world)
{
    Block block = block_from_location(location_random());
    world_set_block(world, &block);
}

static void benchmark_get(World* world)
{
    world_get_block(world, location_random());
}

static void benchmark_delete(World* world)
{
    Block block = block_empty();
    block.location = location_random();
    world_set_block(world, &block);
}

void run_benchmarks(World* world, unsigned int count)
{
    srand(get_time());

    unsigned int limit = MILISECONDS(count);
    assert(limit > 0);

    printf("--- Benchmark Start ---\n");
    BENCHMARK(insert, 200, limit);
    BENCHMARK(get,    200, limit);
    BENCHMARK(delete, 200, limit);
}

