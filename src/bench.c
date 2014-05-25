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
        printf("%s - %lld sec\n", message, time / (1000 * 1000));
    else if (time > 1000 * 1000)
        printf("%s - %lld ms\n", message, time / 1000);
    else
        printf("%s - %lld us\n", message, time);
}

static void block_modified(Block* b) {}

void run_benchmarks(World* world, unsigned int count)
{
    printf("--- Benchmark Start ---\n");
    long long start = get_time();

    BENCHMARK_START(world_add_block)
    CUBE_RANGE(-(int)count, (int)count)
        Location loc = (Location){x,y,z};
        Material mat = (Material)(location_hash_unbounded(loc) % MATERIALS_COUNT);
        Direction dir = (Direction)location_hash(loc, 4);
        unsigned int state = location_hash(loc, 2);
        Block block = block_create(loc, mat, dir, state);
        world_set_block(world, &block);
    CUBE_RANGE_END
    BENCHMARK_END

    BENCHMARK_START(world_get_block)
    CUBE_RANGE(-(int)count, (int)count)
        world_get_block(world, (Location){x, y, z});
    CUBE_RANGE_END
    BENCHMARK_END

    BENCHMARK_START(redstone_tick)
    RANGE(i, 1, count)
        redstone_tick(world, block_modified);
    RANGE_END
    BENCHMARK_END

    WorldStats stats = world_get_stats(world);

    BENCHMARK_START(world_remove_block)
    CUBE_RANGE(-(int)count, (int)count)
        Location loc = (Location){x,y,z};
        Block block = block_create(loc, EMPTY, DIRECTION_DEFAULT, 0);
        world_set_block(world, &block);
    CUBE_RANGE_END
    BENCHMARK_END

    long long end = get_time();
    print_time("total", end - start);
    printf("--- Benchmark End ---\n");

    world_stats_print(stats);
}

