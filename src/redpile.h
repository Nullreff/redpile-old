#ifndef REDPILE_H
#define REDPILE_H

#define REDPILE_VERSION "0.0.1"

#define ERROR_IF(CONDITION, MESSAGE) if (CONDITION) { fprintf(stderr, MESSAGE); exit(EXIT_FAILURE); }
#define CHECK_OOM(POINTER) ERROR_IF(!POINTER, "Out of memory!\n")

typedef struct {
    int world_size;
    int interactive:1;
} RedpileConfig;

#endif
