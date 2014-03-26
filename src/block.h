#ifndef REDPILE_BLOCK_H
#define REDPILE_BLOCK_H

#include "location.h"

typedef enum {
    EMPTY,
    WIRE,
    CONDUCTOR,
    INSULATOR,
    AIR
} Material;

typedef struct {
    Material material;
    Location location;
    int power;
} Block;

#endif
