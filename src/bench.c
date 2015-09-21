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

#include "command.h"
#include "redpile.h"
#include "repl.h"
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
                benchmark_ ## METHOD();\
            }\
            count += PER_ITER;\
            time = get_time() - start;\
        } while (time < LIMIT);\
        float final = count / TO_SECONDS((float)time);\
        if (final > 1000)\
        {\
            final /= 1000;\
            printf(#METHOD ":\t%.2fk / sec\n", final);\
        }\
        else\
        {\
            printf(#METHOD ":\t%.2f / sec\n", final);\
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
static void benchmark_insert(void)
{
    Region region;
    region_randomize(&region, 1);
    char* type_name = indexes[rand() % world->type_data->type_count]->name;
    CommandArgs* args = command_args_allocate(0);
    command_node_set(&region, type_name, args);
    command_args_free(args);
}

static void benchmark_get(void)
{
    Region region;
    region_randomize(&region, 1);
    command_node_get(&region);
}

static void benchmark_delete(void)
{
    Region region;
    region_randomize(&region, 1);
    command_delete(&region);
}

void bench_run(unsigned int count)
{
    indexes = type_data_type_indexes_allocate(world->type_data);
    srand(get_time());

    unsigned int limit = MILISECONDS(count);
    assert(limit > 0);
    repl_mute(true);

    printf("--- Benchmark Start ---\n");
    BENCHMARK(insert, 100, limit);
    BENCHMARK(get,    100, limit);
    BENCHMARK(delete, 100, limit);

    repl_mute(false);
    free(indexes);
}

