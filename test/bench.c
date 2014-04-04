#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"
#include "../src/world.h"
#include "../src/block.h"


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

    printf("%s - %d:%d\n", message, result.tv_sec, result.tv_nsec);
}

int main(int argc, char* argv[])
{
    World* world = malloc(sizeof(World));

    BENCHMARK_START(world_intialize)
    world_intialize(world, 16 * 16 * 16 * 16);
    BENCHMARK_END

    BENCHMARK_START(world_add_block)
    CUBE_RANGE(-16,16)
        Block block = {M_WIRE, (Location){x,y,z}, 0};
        world_add_block(world, &block);
    CUBE_RANGE_END
    BENCHMARK_END

    Block* found_block;
    BENCHMARK_START(world_get_block)
    CUBE_RANGE(-16,16)
        found_block = world_get_block(world, (Location){x, y, z});
    CUBE_RANGE_END
    BENCHMARK_END

    world_print_buckets(world);

    BENCHMARK_START(world_free)
    world_free(world);
    BENCHMARK_END

    free(world);
    return 0;
}
