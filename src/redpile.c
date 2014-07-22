/* redpile.c - High performance redstone
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

#include "redpile.h"
#include "parser.h"
#include "command.h"
#include "bench.h"
#include "type.h"
#include "common.h"
#include <getopt.h>
#include <signal.h>
#include <ctype.h>

// All global state lives in these variables
World* world = NULL;
ScriptState* state = NULL;
RedpileConfig* config = NULL;

int yyparse(void);
int yylex_destroy(void);

static void print_version()
{
    printf("Redpile %s\n", REDPILE_VERSION);
}

static void print_help()
{
    printf("Redpile - High Performance Redstone\n\n"
           "Usage: redpile [options] <config>\n"
           "Options:\n"
           "    -i, --interactive\n"
           "        Run in interactive mode with a prompt for reading commands\n\n"
           "    -w, --world-size\n"
           "        The number of blocks to allocate initially\n\n"
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

    ERROR_IF(*parse_error, "You must pass an integer as the world size\n");
    ERROR_IF(value <= 0, "You must provide a world size larger than zero\n");
    ERROR_IF(!IS_POWER_OF_TWO(value), "You must provide a world size that is a power of two\n");

    return (unsigned int)value;
}

static unsigned int parse_benchmark_size(char* string)
{
    char* parse_error = NULL;
    int value = strtol(string, &parse_error, 10);

    ERROR_IF(*parse_error, "You must pass an integer as the number of benchmarks to run\n");
    ERROR_IF(value <= 0, "You must provide a benchmark size greater than zero\n");

    return (unsigned int)value;
}

static void load_config(int argc, char* argv[])
{
    config = malloc(sizeof(RedpileConfig));

    // Default options
    config->world_size = 1024;
    config->interactive = false;
    config->benchmark = 0;

    static struct option long_options[] =
    {
        {"world-size",  required_argument, NULL, 'w'},
        {"interactive", no_argument,       NULL, 'i'},
        {"version",     no_argument,       NULL, 'v'},
        {"help",        no_argument,       NULL, 'h'},
        {"benchmark",   required_argument, NULL, 'b'},
        {NULL,          0,                 NULL,  0 }
    };

    while (1)
    {
        int opt = getopt_long(argc, argv, "w:ivh", long_options, NULL);
        switch (opt)
        {
            case -1:
                if (optind >= argc)
                {
                    fprintf(stderr, "You must provide a configuration file\n");
                    exit(EXIT_FAILURE);
                }

                config->file = argv[optind];
                return;

            case 'w':
                config->world_size = parse_world_size(optarg);
                break;

            case 'i':
                config->interactive = true;
                break;

            case 'b':
                config->benchmark = parse_benchmark_size(optarg);
                break;

            case 'v':
                print_version();
                exit(EXIT_SUCCESS);

            case 'h':
                print_help();
                exit(EXIT_SUCCESS);

            default:
                exit(EXIT_FAILURE);
        }
    }
}

static void redpile_cleanup(void)
{
    if (world != NULL)
        world_free(world);

    if (state != NULL)
        script_state_free(state);

    if (config != NULL)
        free(config);

    if (!config->benchmark)
        yylex_destroy();

    printf("\n");
}

static void signal_callback(int signal)
{
    if (signal == SIGINT)
    {
        redpile_cleanup();
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char* argv[])
{
    signal(SIGINT, signal_callback);
    load_config(argc, argv);

    state = script_state_allocate();

    TypeData* type_data = script_state_load_config(state, config->file);
    if (type_data == NULL)
    {
        redpile_cleanup();
        return EXIT_FAILURE;
    }

    world = world_allocate(config->world_size, type_data);

    if (config->benchmark)
    {
        run_benchmarks(world, config->benchmark);
    }
    else
    {
        int result;
        do { result = yyparse(); } while (result != 0);
    }

    redpile_cleanup();
    return EXIT_SUCCESS;
}

