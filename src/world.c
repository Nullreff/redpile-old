#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "world.h"
#include "block.h"
#include "redpile.h"
#include "location.h"

void section_initalize(Section* section, Location location)
{
    section->location = location;
    section->blocks = malloc(sizeof(Block) * 16 * 16 * 16);
    CHECK_OOM(section->blocks);
}

void section_free(Section* section)
{
    free(section->blocks);
}

Block* section_get_block(Section* section, Location location)
{
    assert(location.x < 16);
    assert(location.y < 16);
    assert(location.z < 16);
    assert(location.z >= 0);
    assert(location.y >= 0);
    assert(location.z >= 0);
        
    int offset = location.y * 16 * 16 + location.z * 16 + location.x;
    return section->blocks + offset;
}

void world_intialize(World* world)
{
    world->count = 0;
    world->sections = malloc(0);
}

void world_free(World* world)
{
    free(world->sections);
}

void world_initialize_section(World* world, Location location)
{
    Section* temp = realloc(world->sections, sizeof(Section) * (world->count + 1));
    CHECK_OOM(temp);
    world->sections = temp;

    section_initalize(world->sections + world->count, location);
    world->count++;
}

int world_add_block(World* world, Block block)
{
    Block* block_ptr = world_get_block(world, block.location);
    if (block_ptr == NULL)
        return -1;

    *block_ptr = block;
    return 1;
}

Block* world_get_block(World* world, Location location)
{

    // Move this to section coordinates
    Location sec_coords = location;
    sec_coords.x >>= 4;
    sec_coords.y >>= 4;
    sec_coords.z >>= 4;

    int i;
    for (i = 0; i < world->count; i++)
    {
        Location sloc = world->sections[i].location;

        if LOCATION_EQUALS(sloc, sec_coords)
        {
            location.x %= 16;
            location.y %= 16;
            location.z %= 16;
            return section_get_block(world->sections + i, location);
        }
    }

    return NULL;
}

