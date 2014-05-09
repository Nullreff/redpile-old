/* world.c - Data structure for storing a redstone simulation
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

#include "world.h"
#include "block.h"
#include "bucket.h"

World* world_allocate(unsigned int size)
{
    World* world = malloc(sizeof(World));
    CHECK_OOM(world);

    world->ticks = 0;

    world->buckets = bucket_list_allocate(size);
    world->blocks = block_list_allocate(size);
    world->powers = malloc(size * sizeof(int));
    memset(world->powers, EMPTY_INDEX, size * sizeof(int));
    CHECK_OOM(world->powers);

    return world;
}

void world_free(World* world)
{
    bucket_list_free(world->buckets);
    block_list_free(world->blocks);
    free(world->powers);
    free(world);
}

static void world_update_adjacent_blocks(World* world, Block* block)
{
    int index = BLOCK_INDEX(world, block);
    for (int i = 0; i < 6; i++)
    {
        if (block->adjacent[i] == EMPTY_INDEX)
        {
            // Find the bucket next to us
            Direction dir = (Direction)i;
            Location location = location_move(block->location, dir, 1);
            Block* found_block = world_get_block(world, location);

            if (found_block != NULL)
            {
                // Update it to point both ways
                int found_index = BLOCK_INDEX(world, found_block);
                found_block->adjacent[direction_invert(dir)] = index;
                block->adjacent[i] = found_index;
            }
        }
    }
}

Block* world_set_block(World* world, Block* block)
{
    Bucket* bucket = bucket_list_get(world->buckets, block->location, true);

    // Attach the next availabe block to this bucket
    if (!BUCKET_FILLED(bucket))
    {
        bool resized = false;
        int old_size = world->buckets->size;
        bucket->index = block_list_next(world->blocks, &resized);
        if (resized)
        {
            int* temp = realloc(world->powers, world->blocks->size * sizeof(int));
            CHECK_OOM(temp);
            world->powers = temp;
            memset(world->powers, EMPTY_INDEX, world->blocks->size * sizeof(int));
        }
    }

    Block* target = BLOCK_FROM_BUCKET(world, bucket);
    memcpy(target, block, sizeof(Block));

    world_update_adjacent_blocks(world, target);

    return target;
}

Block* world_get_block(World* world, Location location)
{
    Bucket* bucket = bucket_list_get(world->buckets, location, false);
    return BUCKET_FILLED(bucket) ? BLOCK_FROM_BUCKET(world, bucket) : NULL;
}

int world_get_last_power(World* world, Block* block)
{
    int index = block - world->blocks->data;
    int power = world->powers[index];
    return power != EMPTY_INDEX ? power : block->power;
}

void world_set_last_power(World* world, Block* block)
{
    int index = block - world->blocks->data;
    world->powers[index] = block->power;
}

void world_reset_last_power(World* world, int index)
{
    world->powers[index] = EMPTY_INDEX;
}

WorldStats world_get_stats(World* world)
{
    return (WorldStats){
        world->ticks,
        world->blocks->index,
        world->blocks->size,
        world->blocks->resizes,
        world->buckets->size,
        world->buckets->overflow,
        world->buckets->resizes,
        world->buckets->max_depth
    };
}

void world_stats_print(WorldStats stats)
{
    STAT_PRINT(stats, ticks);
    STAT_PRINT(stats, blocks);
    STAT_PRINT(stats, blocks_allocated);
    STAT_PRINT(stats, block_resizes);
    STAT_PRINT(stats, buckets_allocated);
    STAT_PRINT(stats, buckets_overflow);
    STAT_PRINT(stats, buckets_resizes);
    STAT_PRINT(stats, buckets_max_depth);
}

