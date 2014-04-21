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
#include "world.h"
#include "instruction.h"
#include "bench.h"
#include "linenoise.h"
#include <getopt.h>
#include <signal.h>
#include <ctype.h>

RedpileConfig config;
World* world;
char* line;

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
           "    -s, --silent\n"
           "        Don't produce any output from TICK commands.\n\n"
           "    -v, --version\n"
           "        Print the current version\n\n"
           "    -h, --help\n"
           "        Print this message\n\n"
           "    --benchmark\n"
           "        Run benchmarks and stress tests\n");
}

static unsigned int parse_world_size(char* string)
{
    char* parse_error = NULL;
    int value = strtol(string, &parse_error, 10);

    ERROR_IF(*parse_error, "You must pass an integer as the world size\n");
    ERROR_IF(value <= 0, "You must provide a world size larger than zero\n");

    return (unsigned int)value;
}

void load_config(int argc, char* argv[])
{
    // Default options
    config.world_size = 1024;
    config.interactive = 0;
    config.silent = 0;

    static struct option long_options[] =
    {
        {"world-size",  required_argument, NULL, 'w'},
        {"silent",      no_argument,       NULL, 's'},
        {"interactive", no_argument,       NULL, 'i'},
        {"version",     no_argument,       NULL, 'v'},
        {"help",        no_argument,       NULL, 'h'},
        {"benchmark",   no_argument,       NULL, 'b'},
        {NULL,          0,                 NULL,  0 }
    };

    while (1)
    {
        int opt = getopt_long(argc, argv, "w:sivh", long_options, NULL);
        switch (opt)
        {
            case -1:
                return;

            case 'w':
                config.world_size = parse_world_size(optarg);
                break;

            case 's':
                config.silent = 1;
                break;

            case 'i':
                config.interactive = 1;
                break;

            case 'v':
                print_version();
                exit(EXIT_SUCCESS);

            case 'h':
                print_help();
                exit(EXIT_SUCCESS);

            case 'b':
                run_benchmarks();
                exit(EXIT_SUCCESS);

            default:
                exit(EXIT_FAILURE);
        }
    }
}

void redpile_exit(void)
{
    if (world != NULL)
    {
        world_free(world);
    }

    if (line != NULL)
    {
        free(line);
    }

    printf("\n");
    exit(EXIT_SUCCESS);
}

void signal_callback(int signal)
{
    if (signal == SIGINT)
    {
        redpile_exit();
    }
}

void instruction_callback(Block* block)
{
    if (!config.silent)
    {
        block_print_power(block);
    }
}

void completion_callback(const char* buffer, linenoiseCompletions* completions)
{
    for (int i = 0; i < COMMANDS_COUNT; i++)
    {
        bool found = true;
        for (int j = 0; buffer[j] != '\0'; j++)
        {
            if (toupper(buffer[j]) != Commands[i][j])
            {
                found = false;
                break;
            }
        }

        if (found)
        {
            linenoiseAddCompletion(completions, Commands[i]);
        }
    }
}

int main(int argc, char* argv[])
{
    char* prompt;

    signal(SIGINT, signal_callback);
    load_config(argc, argv);
    world = world_allocate(config.world_size);

    if (config.interactive)
    {
        linenoiseSetCompletionCallback(completion_callback);
        prompt = "> ";
    }
    else
    {
        prompt = "";
    }

    Instruction instruction;
    while ((line = linenoise(prompt)) != NULL)
    {
        linenoiseHistoryAdd(line);
        switch (instruction_parse(line, &instruction))
        {
            case 0: // Valid command
                instruction_run(world, &instruction, instruction_callback);
                break;

            case -1: // Parse error
                printf("Invalid Command\n");
                break;

            case -2: // No command
                break;

            case -3: // Exit
                redpile_exit();
        }
        free(line);
    }
}

