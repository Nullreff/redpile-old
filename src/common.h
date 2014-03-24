#ifndef REDPILE_COMMON_H
#define REDPILE_COMMON_H

#define ERROR_IF(CONDITION, MESSAGE) if (CONDITION) { fprintf(stderr, MESSAGE); exit(EXIT_FAILURE); }
#define CHECK_OOM(POINTER) ERROR_IF(!POINTER, "Out of memory!\n")

#endif
