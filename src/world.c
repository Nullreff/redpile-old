/* world.c - Data structure for storing and manipulating blocks
 *
 * Copyright (C) 2014 Ryan Mendivil <ryan@nullreff.net>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "world.h"
#include "block.h"
#include "redpile.h"
#include "bucket.h"

void world_intialize(World* world, unsigned int size)
{
    assert(size > 0);

    world->buckets_size = size;
    world->blocks_size = size;

    world->count = 0;
    world->ticks = 0;
    world->max_depth = 1;
    world->collisions = 0;
    world->power_sources = 0;

    world->buckets = malloc(size * sizeof(Bucket));
    CHECK_OOM(world->buckets);

    world->blocks = malloc(size * sizeof(Block));
    CHECK_OOM(world->blocks);

    int i;
    for (i = 0; i < size; i++)
    {
        world->buckets[i] = bucket_empty();
        world->blocks[i] = block_empty();
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

// Retreives the index of the next available block in the world
int world_next_block(World* world)
{
    if (world->count >= world->blocks_size)
    {
        int new_size = world->blocks_size * 2;

        Block* temp = realloc(world->blocks, new_size * sizeof(Block));
        CHECK_OOM(temp);

        world->blocks_size = new_size;
        world->blocks = temp;

        int i;
        for (i = world->count; i < world->blocks_size; i++)
        {
            world->blocks[i] = block_empty();
        }
    }

    return world->count++;
}

// Finds the bucket used to store the block at the specified location
// If the bucket can't be found and alocate is true, a new bucket and
// block will be created.  If allocate is false, it will return NULL.
Bucket* world_get_bucket(World* world, Location location, bool allocate)
{
    int hash = location_hash(location, world->buckets_size);
    Bucket* bucket = world->buckets + hash;
    int depth = 1;

    if (bucket->index == -1)
    {
        if (allocate)
        {
            bucket->index = world_next_block(world);
        }
        else
        {
            bucket = NULL;
        }
    }
    else
    {
        while (1)
        {
            Block* target = world->blocks + bucket->index;
            if (location_equals(target->location, location))
            {
                break;
            }

            if (bucket->next == NULL)
            {
                if (allocate)
                {
                    bucket_allocate(&bucket->next, world_next_block(world));
                }
                bucket = bucket->next;
                break;
            }

            bucket = bucket->next;
            depth++;
        }
    }

    // Stats tracking
    if (depth > world->max_depth)
    {
        world->max_depth = depth;
    }

    if (depth == 2)
    {
        world->collisions++;
    }

    return bucket;
}

Block* world_add_block(World* world, Block* block)
{
    Bucket* bucket = world_get_bucket(world, block->location, true);
    Block* target = BLOCK_FROM_BUCKET(world, bucket);

    // Update references to nearby buckets
    int i;
    for (i = 0; i < 6; i++)
    {
        if (bucket->adjacent[i] == NULL)
        {
            // Find the bucket next to us
            Direction dir = (Direction)i;
            Location location = location_move(block->location, dir, 1);
            Bucket* found_bucket = world_get_bucket(world, location, false);

            if (found_bucket != NULL)
            {
                // Update it to point both ways
                found_bucket->adjacent[direction_invert(dir)] = bucket;
                bucket->adjacent[i] = found_bucket;
            }
        }
    }

    // Check if we're adding or removing a power source
    if (POWER_SOURCE(block->material))
    {
        if (!POWER_SOURCE(target->material))
        {
            world->power_sources++;
        }
    }
    else
    {
        if (POWER_SOURCE(target->material))
        {
            world->power_sources--;
        }
    }

    memcpy(target, block, sizeof(Block));
    return target;
}

Block* world_get_block(World* world, Location location)
{
    Bucket* bucket = world_get_bucket(world, location, false);
    return BLOCK_FROM_BUCKET(world, bucket);
}

void world_print_status(World* world)
{
    printf("Ticks: %d\n", world->ticks);
    printf("Blocks: %d\n", world->count);
    printf("Power Sources: %d\n", world->power_sources);
    printf("Allocated Blocks: %d\n", world->blocks_size);
    printf("Allocated Buckets: %d\n", world->buckets_size);
    printf("Bucket Collisions: %d\n", world->collisions);
    printf("Max Bucket Depth: %d\n", world->max_depth);
}
