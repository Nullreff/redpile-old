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
#include "world.h"
#include "block.h"
#include "redstone.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

static long long get_time(void)
{
    struct timeval value;
    gettimeofday(&value, NULL);

    long long time = ((long long)value.tv_sec) * 1000000;
    time += value.tv_usec;
    return time;
}

static void print_time(char* message, long long time)
{
    if (time > 1000 * 1000 * 1000)
    {
        printf("%s - %lld sec\n", message, time / (1000 * 1000));
    }
    else if (time > 1000 * 1000)
    {
        printf("%s - %lld ms\n", message, time / 1000);
    }
    else
    {
        printf("%s - %lld us\n", message, time);
    }
}

static void block_modified(Block* b) {}

void run_benchmarks(void)
{
    World* world;

    printf("--- Benchmark Start ---\n");
    long long start = get_time();

    BENCHMARK_START(world_intialize)
    world = world_allocate(1);
    BENCHMARK_END

    BENCHMARK_START(world_add_block)
    CUBE_RANGE(-10,10)
        Location loc = (Location){x,y,z};
        int torch = !location_hash(loc, 20000);
        Block block = block_create(loc, torch ? TORCH : WIRE, UP);
        world_set_block(world, &block);
    CUBE_RANGE_END
    BENCHMARK_END

    BENCHMARK_START(world_get_block)
    CUBE_RANGE(-10,10)
        world_get_block(world, (Location){x, y, z});
    CUBE_RANGE_END
    BENCHMARK_END

    BENCHMARK_START(redstone_tick)
    RANGE(i,1,100)
        redstone_tick(world, block_modified);
    RANGE_END
    BENCHMARK_END

    WorldStats stats = world_get_stats(world);

    BENCHMARK_START(world_free)
    world_free(world);
    BENCHMARK_END

    long long end = get_time();
    printf("--- Benchmark End ---\n");
    print_time("Total Time", end - start);
    world_stats_print(stats);
}

