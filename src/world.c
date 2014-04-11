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
    world->power_sources = 0;

    world->buckets = malloc(size * sizeof(Bucket));
    CHECK_OOM(world->buckets);

    world->blocks = malloc(size * sizeof(Block));
    CHECK_OOM(world->blocks);

    int i;
    for (i = 0; i < size; i++)
    {
        world->buckets[i] = (Bucket){-1, NULL};
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

int world_next_block(World* world, Block** block)
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
            world->blocks[i] = (Block){M_EMPTY, (Location){0, 0, 0}, 0};
        }
    }

    *block = world->blocks + world->count;
    return world->count++;
}

Block* world_add_block(World* world, Block* block)
{
    Bucket* bucket = world_get_bucket(world, block->location);
    Block* target;
    int depth = 1;

    if (bucket->index == -1)
    {
        bucket->index = world_next_block(world, &target);
    }
    else
    {
        while (1)
        {
            target = world->blocks + bucket->index;
            if (location_equals(target->location, block->location))
            {
                break;
            }

            if (bucket->next == NULL)
            {
                bucket->next = malloc(sizeof(Bucket));
                bucket = bucket->next;
                *bucket = (Bucket){world_next_block(world, &target), NULL};
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
    Bucket* bucket = world_get_bucket(world, location);
    Block* target;

    if (bucket->index == -1)
    {
        target = NULL;
    }
    else
    {
        while (1)
        {
            target = world->blocks + bucket->index;
            if (location_equals(target->location, location))
            {
                break;
            }

            if (bucket->next == NULL)
            {
                target = NULL;
                break;
            }

            bucket = bucket->next;
        }
    }

    return target;
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
