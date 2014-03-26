#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include "version.h"
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

void load_config(int argc, char* argv[])
{
    static struct option long_options[] =
    {
        {"interactive", no_argument, NULL, 'i'},
        {"version",     no_argument, NULL, 'v'},
        {"help",        no_argument, NULL, 'h'},
        {NULL,          0,           NULL,  0 }
    };

    int opt = getopt_long(argc, argv, "ivh", long_options, NULL);
    switch (opt)
    {
        case -1:
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

void setup()
{
    world = malloc(sizeof(World));
    world_intialize(world, 256);
}

void cleanup()
{
    if (world != NULL)
    {
        world_free(world);
        free(world);
    }
}

void handle_signal(int signal)
{
    if (signal == SIGINT)
    {
        cleanup();
        exit(EXIT_SUCCESS);
    }
}

void redpile_panic(char* message)
{
    printf(message);
    cleanup();
    exit(EXIT_FAILURE);
}

int read_next_instruction(Instruction* instruction)
{
    char* line = NULL;
    size_t size;
    if (getline(&line, &size, stdin) == -1) {
        cleanup();
        exit(EXIT_SUCCESS);
    }

    if (size == 0)
        return -2;

    // Trim the trailing line return
    size_t ln = strlen(line) - 1;
    if (line[ln] == '\n')
        line[ln] = '\0';

    return instruction_parse(line, instruction);
}

int main(int argc, char* argv[])
{
    load_config(argc, argv);
    signal(SIGINT, handle_signal);
    setup();

    Instruction instruction;
    Block* block;
    while (1)
    {
        if (config.interactive)
        {
            printf("> ");
        }

        int result = read_next_instruction(&instruction);
        if (result == -1)
        {
            printf("Invalid Command\n");
        }
        else if (result != 0)
        {
            continue;
        }

        switch (instruction.cmd)
        {
            case CMD_SET:
                block = &(Block){(Material)instruction.value, instruction.target, 0};
                world_add_block(world, block);
                break;

            case CMD_POWER:
                block = world_get_block(world, instruction.target);
                if (block != NULL)
                {
                    block->power = instruction.value;
                }
                break;

            case CMD_GET:
                block = world_get_block(world, instruction.target);
                break;

            case CMD_TICK:
                printf("Not implemented...\n");
                continue;
        }

        if (block == NULL)
        {
            printf("(%d,%d,%d) EMPTY 0\n",
                    instruction.target.x,
                    instruction.target.y,
                    instruction.target.z);
        }
        else
        {
            printf("(%d,%d,%d) %s %d\n",
                    block->location.x,
                    block->location.y,
                    block->location.z,
                    Materials[block->material],
                    block->power);
        }
    }

    cleanup();
    return EXIT_SUCCESS;
}

