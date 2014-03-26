#ifndef REDPILE_BLOCK_H
#define REDPILE_BLOCK_H

#include "location.h"

char* Materials[5];

typedef enum {
    EMPTY,
    AIR,
    WIRE,
    CONDUCTOR,
    INSULATOR
} Material;

typedef struct {
    Material material;
    Location location;
    int power;
} Block;

#endif
