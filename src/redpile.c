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
#include "world.h"
#include "redstone.h"
#include "bench.h"
#include "linenoise.h"
#include "type.h"
#include <getopt.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>

RedpileConfig config;
char* prompt;

int yyparse(void);
int yylex_destroy(void);

static void print_version()
{
    printf("Redpile %s\n", REDPILE_VERSION);
}

static void print_help()
{
    printf("Redpile - High Performance Redstone\n\n"
           "Usage: redpile [options] [map directory]\n"
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
    // Default options
    config.world_size = 1024;
    config.interactive = 0;
    config.benchmark = 0;

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
                return;

            case 'w':
                config.world_size = parse_world_size(optarg);
                break;

            case 'i':
                config.interactive = 1;
                break;

            case 'b':
                config.benchmark = parse_benchmark_size(optarg);
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

#define TYPE_COUNT 9
static TypeList* load_types(void)
{
    TypeList* types = type_list_allocate(TYPE_COUNT);

    BehaviorList* air_behaviors = behavior_list_allocate(0);
    types->data[0] = (Type){"AIR", 0, air_behaviors};

    BehaviorList* insulator_behaviors = behavior_list_allocate(1);
    insulator_behaviors->data[0].process = BEHAVIOR(PUSH_MOVE);
    types->data[1] = (Type){"INSULATOR", 0, insulator_behaviors};

    BehaviorList* wire_behaviors = behavior_list_allocate(2);
    wire_behaviors->data[0].process = BEHAVIOR(PUSH_BREAK);
    wire_behaviors->data[1].process = BEHAVIOR(WIRE);
    types->data[2] = (Type){"WIRE", 1, wire_behaviors};

    BehaviorList* conductor_behaviors = behavior_list_allocate(2);
    conductor_behaviors->data[0].process = BEHAVIOR(PUSH_MOVE);
    conductor_behaviors->data[1].process = BEHAVIOR(CONDUCTOR);
    types->data[3] = (Type){"CONDUCTOR", 1, conductor_behaviors};

    BehaviorList* torch_behaviors = behavior_list_allocate(2);
    torch_behaviors->data[0].process = BEHAVIOR(PUSH_BREAK);
    torch_behaviors->data[1].process = BEHAVIOR(TORCH);
    types->data[4] = (Type){"TORCH", 2, torch_behaviors};

    BehaviorList* piston_behaviors = behavior_list_allocate(2);
    piston_behaviors->data[0].process = BEHAVIOR(PISTON);
    piston_behaviors->data[1].process = BEHAVIOR(PUSH_MOVE);
    types->data[5] = (Type){"PISTON", 2, piston_behaviors};

    BehaviorList* repeater_behaviors = behavior_list_allocate(2);
    repeater_behaviors->data[0].process = BEHAVIOR(PUSH_BREAK);
    repeater_behaviors->data[1].process = BEHAVIOR(REPEATER);
    types->data[6] = (Type){"REPEATER", 3, repeater_behaviors};

    BehaviorList* comparator_behaviors = behavior_list_allocate(2);
    comparator_behaviors->data[0].process = BEHAVIOR(PUSH_BREAK);
    comparator_behaviors->data[1].process = BEHAVIOR(COMPARATOR);
    types->data[7] = (Type){"COMPARATOR", 3, comparator_behaviors};

    BehaviorList* switch_behaviors = behavior_list_allocate(2);
    switch_behaviors->data[0].process = BEHAVIOR(PUSH_BREAK);
    switch_behaviors->data[1].process = BEHAVIOR(SWITCH);
    types->data[8] = (Type){"SWITCH", 3, switch_behaviors};

    return types;
}

static void redpile_cleanup(void)
{
    if (current_world != NULL)
        world_free(current_world);

    if (!config.benchmark)
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

int read_input(char *buff, int buffsize)
{
    if (!config.interactive)
        return read(STDIN_FILENO, buff, buffsize);

    char* line = linenoise("> ");
    if (line == NULL)
        return 0;

    // Linenoise has a buffer size of 4096
    // Flex has a default buffer size of at least 8192 on 32 bit
    int size = strlen(line);
    if (size + 2 > buffsize)
        fprintf(stderr, "Line too long, truncating to %i\n", buffsize);

    // Flex won't generate output until we fill it's buffer
    // Since this is interactive mode, we just zero it out
    // and fill it with whatever we read in.
    memset(buff, '\0', buffsize);
    memcpy(buff, line, size);

    // Linenoise strips out the line return
    buff[size]     = '\n';

    free(line);
    return buffsize;
}

int main(int argc, char* argv[])
{
    signal(SIGINT, signal_callback);
    load_config(argc, argv);
    current_world = world_allocate(config.world_size, load_types());

    if (config.benchmark)
    {
        run_benchmarks(current_world, config.benchmark);
    }
    else
    {
        int result;
        do { result = yyparse(); } while (result != 0);
    }

    redpile_cleanup();
    return EXIT_SUCCESS;
}

