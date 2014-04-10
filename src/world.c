#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "world.h"
#include "block.h"
#include "redpile.h"

void world_intialize(World* world, unsigned int size)
{
    assert(size > 0);

    world->buckets_size = size;
    world->blocks_size = size;

    world->count = 0;
    world->ticks = 0;
    world->max_depth = 1;
    world->collisions = 0;

    world->buckets = malloc(size * sizeof(Bucket));
    CHECK_OOM(world->buckets);

    world->blocks = malloc(size * sizeof(Block));
    CHECK_OOM(world->blocks);

    int i;
    for (i = 0; i < size; i++)
    {
        world->buckets[i] = (Bucket){NULL, NULL};
        world->blocks[i] = (Block){M_EMPTY, (Location){0, 0, 0}, 0, 0};
    }
}

void world_free(World* world)
{
    Bucket* bucket;
    Bucket* next;

    int i;
    for (i = 0; i < world->buckets_size; i++)
    {
        bucket = world->buckets[i].next;
        while (bucket != NULL)
        {
            next = bucket->next;
            free(bucket);
            bucket = next;
        }
    }

    free(world->buckets);
    free(world->blocks);
}

Bucket* world_get_bucket(World* world, Location location)
{
    int hash = location_hash(location, world->buckets_size);
    return world->buckets + hash;
}

Block* world_next_block(World* world)
{
    if (world->count >= world->blocks_size)
    {
        Block* temp = realloc(world->blocks, world->blocks_size * 2 * sizeof(Block));
        CHECK_OOM(temp);

        world->blocks_size *= 2;
        world->blocks = temp;

        int i;
        for (i = world->count; i < world->blocks_size; i++)
        {
            world->blocks[i] = (Block){M_EMPTY, (Location){0, 0, 0}, 0};
        }
    }

    Block* block = world->blocks + world->count;
    world->count++;
    return block;
}

Block* world_add_block(World* world, Block* block)
{
    Bucket* bucket = world_get_bucket(world, block->location);
    int depth = 1;

    if (bucket->block == NULL)
    {
        bucket->block = world_next_block(world);
    }
    else
    {
        while (!location_equals(bucket->block->location, block->location))
        {
            if (bucket->next == NULL)
            {
                bucket->next = malloc(sizeof(Bucket));
                bucket = bucket->next;
                *bucket = (Bucket){world_next_block(world), NULL};
                break;
            }

            bucket = bucket->next;
            depth++;
        }
    }

    if (depth > world->max_depth)
    {
        world->max_depth = depth;
    }

    if (depth == 2)
    {
        world->collisions++;
    }

    memcpy(bucket->block, block, sizeof(Block));
    return bucket->block;
}

Block* world_get_block(World* world, Location location)
{
    Bucket* bucket = world_get_bucket(world, location);

    if (bucket->block == NULL)
    {
        return NULL;
    }

    while (1)
    {
        if (location_equals(bucket->block->location, location))
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

void world_print_status(World* world)
{
    printf("Ticks: %d\n", world->ticks);
    printf("Blocks: %d\n", world->count);
    printf("Allocated Blocks: %d\n", world->blocks_size);
    printf("Allocated Buckets: %d\n", world->buckets_size);
    printf("Bucket Collisions: %d\n", world->collisions);
    printf("Max Bucket Depth: %d\n", world->max_depth);
}
