/* redpile.c - Voxel logic simulator
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

#include "redpile.h"
#include "parser.h"
#include "command.h"
#include "bench.h"
#include "type.h"
#include "common.h"
#include "repl.h"
#include <getopt.h>
#include <signal.h>
#include <ctype.h>

// All global state lives in these variables
World* world = NULL;
ScriptState* state = NULL;
RedpileConfig* config = NULL;

static void print_version()
{
}

static void print_help()
{
    printf("Redpile - A Voxel Logic Simulator\n\n"
           "Usage: redpile [options] <config>\n"
           "Options:\n"
           "    -p <port>, --port <port>\n"
           "        Listen on the specified port for commands\n\n"
           "    -i, --interactive\n"
           "        Run in interactive mode with a prompt for reading commands\n\n"
           "    -w <size>, --world-size <size>\n"
           "        The number of nodes to allocate initially\n\n"
           "    -v, --version\n"
           "        Print the current version\n\n"
           "    -h, --help\n"
           "        Print this message\n\n"
           "    --benchmark <milliseconds>\n"
           "        Run each benchmark for the time specified\n");
}

static unsigned int parse_world_size(char* string)
{
    char* parse_error = NULL;
    int value = strtol(string, &parse_error, 10);

    if (*parse_error)
    {
        WARN("You must pass an integer as the world size\n");
        value = 0;
    }
    else if (value <= 0)
    {
        WARN("You must provide a world size larger than zero\n");
        value = 0;
    }
    else if (!IS_POWER_OF_TWO(value))
    {
        WARN("You must provide a world size that is a power of two\n");
        value = 0;
    }

    return (unsigned int)value;
}

static unsigned int parse_benchmark_size(char* string)
{
    char* parse_error = NULL;
    int value = strtol(string, &parse_error, 10);

    if (*parse_error)
    {
        WARN("You must pass an integer as the number of benchmarks to run\n");
        value = 0;
    }
    else if (value <= 0)
    {
        WARN("You must provide a benchmark size greater than zero\n");
        value = 0;
    }

    return (unsigned int)value;
}

static unsigned short parse_port_number(char* string)
{
    char* parse_error = NULL;
    int value = strtol(string, &parse_error, 10);

    if (*parse_error)
    {
        WARN("You must pass an integer as the port number\n");
        value = 0;
    }
    else if (value <= 0)
    {
        WARN("You must provide a port number greater than zero\n");
        value = 0;
    }
    else if (value > USHRT_MAX)
    {
        WARN("You must provide a port number less than or equal to %d\n", USHRT_MAX);
        value = 0;
    }

    return (unsigned short)value;
}

#define CONFIG_SUCCESS 0
#define CONFIG_FAIL 1
#define CONFIG_EXIT 2
static int load_config(int argc, char* argv[])
{
    config = malloc(sizeof(RedpileConfig));

    // Default options
    config->world_size = 1024;
    config->interactive = false;
    config->port = 0;
    config->benchmark = 0;

    static struct option long_options[] =
    {
        {"world-size",  required_argument, NULL, 'w'},
        {"interactive", no_argument,       NULL, 'i'},
        {"port",        required_argument, NULL, 'p'},
        {"version",     no_argument,       NULL, 'v'},
        {"help",        no_argument,       NULL, 'h'},
        {"benchmark",   required_argument, NULL, 'b'},
        {NULL,          0,                 NULL,  0 }
    };

    while (1)
    {
        int opt = getopt_long(argc, argv, "w:ip:vh", long_options, NULL);
        switch (opt)
        {
            case -1:
                if (optind >= argc)
                {
                    WARN("You must provide a configuration file\n");
                    return CONFIG_FAIL;
                }

                config->file = argv[optind];
                return CONFIG_SUCCESS;

            case 'w':
                config->world_size = parse_world_size(optarg);
                if (config->world_size == 0)
                    return CONFIG_FAIL;
                break;

            case 'i':
                config->interactive = true;
                break;

            case 'p':
                config->port = parse_port_number(optarg);
                if (config->port == 0)
                    return CONFIG_FAIL;
                break;

            case 'b':
                config->benchmark = parse_benchmark_size(optarg);
                if (config->benchmark == 0)
                    return CONFIG_FAIL;
                break;

            case 'v':
                print_version();
                return CONFIG_EXIT;

            case 'h':
                print_help();
                return CONFIG_EXIT;

            default:
                return CONFIG_FAIL;
        }
    }
}

static void signal_callback(int signal)
{
    if (signal == SIGINT)
    {
        redpile_cleanup();
        exit(EXIT_SUCCESS);
    }
}

void setup_signals(void)
{
    signal(SIGINT, signal_callback);
}

// Referenced from common.h
void redpile_cleanup(void)
{
    if (world != NULL)
        world_free(world);

    if (state != NULL)
        script_state_free(state);

    if (config != NULL)
        free(config);

    repl_cleanup();

    printf("\n");
}

int redpile_run(int argc, char** argv)
{
    setup_signals();
    int result = load_config(argc, argv);
    if (result != CONFIG_SUCCESS)
    {
        redpile_cleanup();
        if (result == CONFIG_FAIL)
            return EXIT_FAILURE;
        else
            return EXIT_SUCCESS;
    }

    state = script_state_allocate();

    TypeData* type_data = script_state_load_config(state, config->file);
    if (type_data == NULL)
    {
        redpile_cleanup();
        return EXIT_FAILURE;
    }

    world = world_allocate(config->world_size, type_data);

    if (config->benchmark)
        bench_run(world, config->benchmark);
    else
        repl_run();

    redpile_cleanup();
    return EXIT_SUCCESS;
}

