#ifndef REDPILE_WORLD_H
#define REDPILE_WORLD_H

#include "block.h"

typedef struct Bucket {
    Block* block;
    struct Bucket* next;
} Bucket;

typedef struct {
    // Block storage/lookup using a basic hashmap
    // Blocks are placed sequentially in `blocks`
    // with a bucket added to `buckets` for lookup
    int buckets_size;
    int blocks_size;
    Bucket* buckets;
    Block* blocks;

    // Stats used by `world_print_status`
    int count;      // Total blocks
    int ticks;      // Redstone ticks
    int max_depth;  // Deepest bucket
    int collisions; // Number of hash collisions
} World;

void world_intialize(World* world, unsigned int size);
void world_free(World* world);
Block* world_add_block(World* world, Block* block);
Block* world_get_block(World* world, Location location);
void world_print_status(World* world);

#endif

