#ifndef REDPILE_WORLD_H
#define REDPILE_WORLD_H

#include "location.h"
#include "block.h"
#include "instruction.h"

typedef struct BlockBucket {
    Block* block;
    struct BlockBucket* next;
} BlockBucket;

typedef struct {
    int size;
    BlockBucket* blocks;  
} World;

void world_intialize(World* world, int size);
void world_free(World* world);
void world_add_block(World* world, Block* block);
Block* world_get_block(World* world, Location location);

#endif

