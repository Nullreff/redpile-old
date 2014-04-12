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

void redstone_wire_update(World* world, Bucket* bucket)
{
    Block* block = BLOCK_FROM_BUCKET(world, bucket);
    block->updated = 1;

    Direction directions[4] = {NORTH, SOUTH, EAST, WEST};

    int i;
    for (i = 0; i < 4; i++)
    {
        Bucket* adjacent = bucket->adjacent[directions[i]];
        if (!BUCKET_FILLED(adjacent))
            continue;

        Block* found_block = BLOCK_FROM_BUCKET(world, adjacent);
        if (found_block != NULL && found_block->material == WIRE)
        {
            if (!found_block->updated || found_block->power < (block->power - 1))
            {
                world_set_last_power(world, adjacent);
                found_block->power = block->power - 1;
                redstone_wire_update(world, adjacent);
            }
        }
        // TODO: Propigate to more materials
    }
}

void redstone_torch_update(World* world, Bucket* bucket)
{
    Block* block = BLOCK_FROM_BUCKET(world, bucket);
    block->updated = 1;

    // Set the torch power from the block behind it
    Bucket* power_source = bucket->adjacent[direction_invert(block->direction)];
    if (power_source != NULL)
    {
        int power = world_get_last_power(world, power_source);
        block->power = power == 0 ? 15 : 0;
    }
    else
    {
        block->power = 15;
    }

    Direction directions[5] = {NORTH, SOUTH, EAST, WEST, DOWN};

    int i;
    for (i = 0; i < 5; i++)
    {
        Bucket* adjacent = bucket->adjacent[directions[i]];
        if (!BUCKET_FILLED(adjacent))
            continue;

        Block* found_block = BLOCK_FROM_BUCKET(world, adjacent);
        if (found_block != NULL && found_block->material == WIRE)
        {
            world_set_last_power(world, adjacent);
            found_block->power = block->power;
            redstone_wire_update(world, adjacent);
        }
        // TODO: Propigate to more materials
    }
}

void redstone_tick(World* world, void (*block_modified_callback)(Block*))
{
    int i;

    // Process all power sources
    for (i = 0; i < world->buckets_size; i++)
    {
        Bucket* bucket = world->buckets + i;
        while BUCKET_FILLED(bucket)
        {
            Block* block = BLOCK_FROM_BUCKET(world, bucket);

            if (block != NULL && !block->updated)
            {
                switch (block->material)
                {
                    // Add more powers sources here as needed
                    case TORCH:
                        redstone_torch_update(world, bucket);
                }
            }

            bucket = bucket->next;
        }
    }

    // Check for block modifications and reset flags
    for (i = 0; i < world->blocks_size; i++)
    {
        Block* block = world->blocks + i;

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
        world->old_powers[i] = -1;
    }

    world->ticks++;
}

