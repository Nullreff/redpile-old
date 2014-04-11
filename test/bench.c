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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"
#include "../src/world.h"
#include "../src/block.h"
#include "../src/redstone.h"


#define BENCHMARK_START(name) do {\
    struct timespec start, end;\
    char* message = #name;\
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

#define BENCHMARK_END\
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);\
    print_time(message, start, end);\
} while(0);

void print_time(char* message, struct timespec start, struct timespec end)
{
    struct timespec result;

    if ((end.tv_nsec - start.tv_nsec) < 0)
    {
        result.tv_sec = end.tv_sec - start.tv_sec - 1;
        result.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    }
    else
    {
        result.tv_sec = end.tv_sec - start.tv_sec;
        result.tv_nsec = end.tv_nsec - start.tv_nsec;
    }

    printf("%s - %ld:%ld\n", message, result.tv_sec, result.tv_nsec);
}

void block_modified(Block* b) {}

int main(int argc, char* argv[])
{
    World* world = malloc(sizeof(World));

    BENCHMARK_START(world_intialize)
    world_intialize(world, 10 * 10 * 10);
    BENCHMARK_END

    BENCHMARK_START(world_add_block)
    CUBE_RANGE(-10,10)
        Location loc = (Location){x,y,z};
        int torch = !location_hash(loc, 20000);
        Block block = {torch ? M_TORCH : M_WIRE, loc, 0, 0};
        world_add_block(world, &block);
    CUBE_RANGE_END
    BENCHMARK_END

    Block* found_block;
    BENCHMARK_START(world_get_block)
    CUBE_RANGE(-10,10)
        found_block = world_get_block(world, (Location){x, y, z});
    CUBE_RANGE_END
    BENCHMARK_END

    BENCHMARK_START(redstone_tick)
    RANGE(i,1,10 * 10 * 10)
    {
        redstone_tick(world, block_modified);
    }
    BENCHMARK_END

    world_print_status(world);

    BENCHMARK_START(world_free)
    world_free(world);
    BENCHMARK_END

    free(world);
    return 0;
}
