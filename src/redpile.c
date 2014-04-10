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

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include "redpile.h"
#include "world.h"
#include "instruction.h"

RedpileConfig config;
World* world;

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
           "    -v, --version\n"
           "        Print the current version\n\n"
           "    -h, --help\n"
           "        Print this message\n");
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
    config.interactive = 0;
    config.world_size = 1024;

    static struct option long_options[] =
    {
        {"world-size",  required_argument, NULL, 's'},
        {"interactive", no_argument,       NULL, 'i'},
        {"version",     no_argument,       NULL, 'v'},
        {"help",        no_argument,       NULL, 'h'},
        {NULL,          0,                 NULL,  0 }
    };

    while (1)
    {
        int opt = getopt_long(argc, argv, "s:ivh", long_options, NULL);
        switch (opt)
        {
            case -1:
                return;

            case 's':
                config.world_size = parse_world_size(optarg);
                break;

            case 'v':
                print_version();
                exit(EXIT_SUCCESS);

            case 'h':
                print_help();
                exit(EXIT_SUCCESS);

            case 'i':
                config.interactive = 1;
                break;

            default:
                exit(EXIT_FAILURE);
        }
    }
}

void setup()
{
    world = malloc(sizeof(World));
    world_intialize(world, config.world_size);
}

void cleanup()
{
    if (world != NULL)
    {
        world_free(world);
        free(world);
    }
    printf("\n");
}

void handle_signal(int signal)
{
    if (signal == SIGINT)
    {
        cleanup();
        exit(EXIT_SUCCESS);
    }
}

int read_next_instruction(Instruction* instruction)
{
    char* line = NULL;
    int result;
    size_t size;

    if (getline(&line, &size, stdin) == EOF)
    {
        result = -3;
        goto cleanup;
    }

    if (size == 0)
    {
        result = -2;
        goto cleanup;
    }

    // Trim the trailing line return
    size_t ln = strlen(line) - 1;
    if (line[ln] == '\n')
    {
        line[ln] = '\0';
    }

    result = instruction_parse(line, instruction);

cleanup:
    free(line);
    return result;
}

void instruction_callback(Block* block)
{
    printf("(%d,%d,%d) %s %d\n",
            block->location.x,
            block->location.y,
            block->location.z,
            Materials[block->material],
            block->power);
}

int main(int argc, char* argv[])
{
    load_config(argc, argv);
    signal(SIGINT, handle_signal);
    setup();

    Instruction instruction;
    while (1)
    {
        if (config.interactive)
        {
            printf("> ");
        }

        int result = read_next_instruction(&instruction);
        switch (result)
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
                cleanup();
                return EXIT_SUCCESS;
        }
    }
}

