#ifndef REDPILE_WORLD_H
#define REDPILE_WORLD_H

#include "location.h"
#include "block.h"
#include "instruction.h"

// Sections are areas of 16x16x16 blocks
typedef struct {
    Location location;
    Block* blocks;
} Section;

typedef struct {
    int count;
    Section* sections;  
} World;

void world_intialize(World* world);
void world_free(World* world);
void world_initialize_section(World* world, Location location);
int world_add_block(World* world, Block block);
Block* world_get_block(World* world, Location location);

#endif

