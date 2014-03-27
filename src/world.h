#ifndef REDPILE_WORLD_H
#define REDPILE_WORLD_H

#include "block.h"

typedef struct Bucket {
    Block* block;
    struct Bucket* next;
} Bucket;

typedef struct {
    int count;
    int buckets_size;
    int blocks_size;
    Bucket* buckets;
    Block* blocks;
} World;

void world_intialize(World* world, int size);
void world_free(World* world);
Block* world_add_block(World* world, Block* block);
Block* world_get_block(World* world, Location location);
void world_run_tick(World* world, void(*block_modified_callback)(Block*));
void world_print_buckets(World* world);

#endif

