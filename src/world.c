#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "world.h"
#include "block.h"
#include "redpile.h"
#include "location.h"

void world_intialize(World* world, int size)
{
    assert(size > 0);
    world->size = size;
    world->blocks = malloc(size * sizeof(BlockBucket));

    int i;
    for (i = 0; i < size; i++)
    {
        world->blocks[i] = (BlockBucket){NULL, NULL};
    }
}

void world_free(World* world)
{
    BlockBucket* bucket;
    BlockBucket* next;

    int i;
    for (i = 0; i < world->size; i++)
    {
        if (world->blocks[i].block != NULL)
        {
            free(world->blocks[i].block);
        }
        bucket = world->blocks[i].next;
        while (bucket != NULL)
        {
            free(bucket->block);
            next = bucket->next;
            free(bucket);
            bucket = next;
        }
    }
    free(world->blocks);
}

BlockBucket* world_get_bucket(World* world, Location location)
{
    long hash = LOCATION_HASH(location, world->size);
    return world->blocks + hash;
}

void world_add_block(World* world, Block* block)
{
    BlockBucket* bucket = world_get_bucket(world, block->location);

    if (bucket->block == NULL)
    {
        goto block;
    }

    while (1)
    {
        if LOCATION_EQUALS(bucket->block->location, block->location)
        {
            goto copy;
        }

        if (bucket->next == NULL)
        {
            goto bucket;
        }

        bucket = bucket->next;
    }

bucket:
    bucket->next = malloc(sizeof(BlockBucket));
    bucket = bucket->next;
    *bucket = (BlockBucket){NULL, NULL};
block:
    bucket->block = malloc(sizeof(Block));
copy:
    memcpy(bucket->block, block, sizeof(Block));
}

Block* world_get_block(World* world, Location location)
{
    BlockBucket* bucket = world_get_bucket(world, location);

    if (bucket->block == NULL)
    {
        return NULL;
    }

    while (1)
    {
        if LOCATION_EQUALS(bucket->block->location, location)
        {
            return bucket->block;
        }

        if (bucket->next == NULL)
        {
            return NULL;
        }

        bucket = bucket->next;
    }
}

