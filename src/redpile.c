#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <signal.h>
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

int main(int argc, char* argv[])
{
    load_config(argc, argv);
    signal(SIGINT, handle_signal);
    setup();

    if (config.interactive)
    {
        printf("Running in interactive mode\n");
    }
    else
    {
        printf("Running in normal mode\n");
    }

    cleanup();
}

