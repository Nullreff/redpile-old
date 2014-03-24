#ifndef REDPILE_BLOCK_H
#define REDPILE_BLOCK_H

#include "location.h"

typedef enum {
    WIRE,
    CONDUCTOR,
    INSULATOR,
    AIR
} Material;

typedef struct {
    Location location;
    Material material;
    int power;
} Block;

#endif
