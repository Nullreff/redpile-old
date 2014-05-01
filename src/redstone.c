/* redstone.c - Redstone logic implementation
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

#include <string.h>
#include "world.h"
#include "block.h"
#include "redstone.h"

Bucket* _bucket;

#define FIND_BLOCK(bucket,dir)\
    (_bucket = BUCKET_ADJACENT(bucket, dir),\
    (_bucket != NULL ? BLOCK_FROM_BUCKET(world, _bucket) : NULL))

#define MOVE_TO_BLOCK(bucket,dir)\
    (bucket = BUCKET_ADJACENT(bucket, dir),\
    (bucket != NULL ? BLOCK_FROM_BUCKET(world, bucket) : NULL))

#define SHOULD_UPDATE(block,new_power)\
    (!(block)->updated || (block)->power < (new_power))

void redstone_wire_update(World* world, Bucket* bucket);

void redstone_wire_update_adjacent(World* world, Bucket* bucket, Block* block, Direction dir, bool covered)
{
    // Directly adjacent
    Bucket* adjacent = BUCKET_ADJACENT(bucket, dir);
    if (adjacent == NULL)
        return;

    Block* found_block = BLOCK_FROM_BUCKET(world, adjacent);

    // Down one block
    if (found_block->material == EMPTY || found_block->material == AIR)
    {
        found_block = MOVE_TO_BLOCK(adjacent, DOWN);
    }
    // Up one block
    else if (found_block->material == CONDUCTOR)
    {
        // Add charge to the block
        Block* right = FIND_BLOCK(bucket, direction_left(dir));
        Block* left = FIND_BLOCK(bucket, direction_right(dir));

        if ((right == NULL || right->material != WIRE) &&
            (left == NULL || left->material != WIRE) &&
            SHOULD_UPDATE(found_block, block->power))
        {
            world_set_last_power(world, adjacent);
            found_block->power = block->power;
            found_block->updated = 1;
        }

        if (covered)
            return;

        found_block = MOVE_TO_BLOCK(adjacent, UP);
    }

    if (found_block == NULL || found_block->material != WIRE)
        return;

    int new_power = block->power - 1;
    if SHOULD_UPDATE(found_block, new_power)
    {
        world_set_last_power(world, adjacent);
        found_block->power = new_power;
        redstone_wire_update(world, adjacent);
    }
}

void redstone_wire_update(World* world, Bucket* bucket)
{
    Block* block = BLOCK_FROM_BUCKET(world, bucket);
    block->updated = 1;

    Block* above = FIND_BLOCK(bucket, UP);
    bool covered = above != NULL &&
                   above->material != AIR &&
                   above->material != EMPTY;

    // Adjacent blocks
    Direction directions[4] = {NORTH, SOUTH, EAST, WEST};
    for (int i = 0; i < 4; i++)
        redstone_wire_update_adjacent(world, bucket, block, directions[i], covered);

    // Block below
    Bucket* down_bucket = BUCKET_ADJACENT(bucket, DOWN);
    if (down_bucket != NULL)
    {
        Block* down_block = BLOCK_FROM_BUCKET(world, down_bucket);
        if (down_block->material == CONDUCTOR && SHOULD_UPDATE(down_block, block->power))
        {
            world_set_last_power(world, down_bucket);
            down_block->power = block->power;
            down_block->updated = 1;
        }
    }
}

void redstone_torch_update(World* world, Bucket* bucket)
{
    Block* block = BLOCK_FROM_BUCKET(world, bucket);
    block->updated = 1;

    // Set the torch power from the block behind it
    Direction behind = direction_invert(block->direction);
    Bucket* power_source = BUCKET_ADJACENT(bucket, behind);
    if (power_source != NULL)
    {
        int power = world_get_last_power(world, power_source);
        block->power = power == 0 ? 15 : 0;
    }
    else
    {
        block->power = 15;
    }

    // Pass charge to any adjacent wires
    Direction directions[5] = {NORTH, SOUTH, EAST, WEST, DOWN};
    for (int i = 0; i < 5; i++)
    {
        if (directions[i] == behind)
            continue;

        Bucket* adjacent = BUCKET_ADJACENT(bucket, directions[i]);
        if (!BUCKET_FILLED(adjacent))
            continue;

        Block* found_block = BLOCK_FROM_BUCKET(world, adjacent);
        if (found_block == NULL)
            continue;

        if (found_block->material == WIRE)
        {
            world_set_last_power(world, adjacent);
            found_block->power = block->power;
            redstone_wire_update(world, adjacent);
        }
    }

    // Pass charge up through a block
    do
    {
        Bucket* up_bucket = BUCKET_ADJACENT(bucket, UP);
        if (up_bucket == NULL)
            break;

        Block* up_block = BLOCK_FROM_BUCKET(world, up_bucket);
        if (up_block->material != CONDUCTOR)
            break;

        world_set_last_power(world, up_bucket);
        up_block->power = block->power;
        up_block->updated = 1;

        Bucket* up_2_bucket = BUCKET_ADJACENT(up_bucket, UP);
        if (up_2_bucket == NULL)
            break;

        Block* up_2_block = BLOCK_FROM_BUCKET(world, up_2_bucket);
        if (up_2_block->material != WIRE)
            break;

        world_set_last_power(world, up_2_bucket);
        up_2_block->power = block->power;
        redstone_wire_update(world, up_2_bucket);
    } while (0);
}

void redstone_tick(World* world, void (*block_modified_callback)(Block*))
{
    // Process all power sources
    for (int i = 0; i < world->buckets->index; i++)
    {
        Bucket* bucket = world->buckets->data + i;
        if (!BUCKET_FILLED(bucket))
            continue;

        Block* block = BLOCK_FROM_BUCKET(world, bucket);
        if (block == NULL || block->updated)
            continue;

        if POWER_SOURCE(block->material)
            redstone_torch_update(world, bucket);
    }

    // Check for block modifications and reset flags
    for (int i = 0; i < world->blocks->index; i++)
    {
        Block* block = world->blocks->data + i;

        if (block->updated)
        {
            block_modified_callback(block);
            block->updated = 0;
        }
        else if (block->power > 0)
        {
            // Blocks not connected to a power source become unpowered
            block->power = 0;
            block_modified_callback(block);
        }

        // Reset old power values
        world->powers[i] = -1;
    }

    world->ticks++;
}

