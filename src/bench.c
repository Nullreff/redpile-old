/* bench.c - Redpile benchmarks and stress tests
 *
 * Copyright (C) 2014 Ryan Mendivil <ryan@nullreff.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redpile nor the names of its contributors may be
 *     used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "bench.h"
#include "tick.h"
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

Type** indexes;
static void benchmark_insert(World* world)
{
    Type* type = indexes[rand() % world->type_data->type_count];
    world_set_node(world, location_random(), type, NULL);
}

static void benchmark_get(World* world)
{
    Node node;
    world_get_node(world, location_random(), &node);
}

static void benchmark_delete(World* world)
{
    world_remove_node(world, location_random());
}

void bench_run(World* world, unsigned int count)
{
    indexes = type_data_type_indexes_allocate(world->type_data);
    srand(get_time());

    unsigned int limit = MILISECONDS(count);
    assert(limit > 0);

    printf("--- Benchmark Start ---\n");
    BENCHMARK(insert, 200, limit);
    BENCHMARK(get,    200, limit);
    BENCHMARK(delete, 200, limit);

    free(indexes);
}

