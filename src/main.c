#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "version.h"

static struct option long_options[] =
{
    {"interactive", no_argument, NULL, 'i'},
    {"version",     no_argument, NULL, 'v'},
    {"help",        no_argument, NULL, 'h'},
    {NULL,          0,           NULL,  0 }
};

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

int main(int argc, char* argv[])
{
    int interactive = 0;

    int opt = getopt_long(argc, argv, "ivh", long_options, NULL);
    {
        switch (opt)
        {
            case -1:
                break;
            case 'i':
                interactive = 1;
                break;
            case 'v':
                print_version();
                return EXIT_SUCCESS;
            case 'h':
                print_help();
                return EXIT_SUCCESS;
            default:
                return EXIT_FAILURE;
        }
    }

    if (interactive)
    {
        printf("Running in interactive mode\n");
    }
    else
    {
        printf("Running in normal mode\n");
    }
}

