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

    Bucket* buckets[4];
    buckets[0] = bucket->adjacent[D_NORTH];
    buckets[1] = bucket->adjacent[D_SOUTH];
    buckets[2] = bucket->adjacent[D_EAST];
    buckets[3] = bucket->adjacent[D_WEST];

    int i;
    for (i = 0; i < 4; i++)
    {
        if (!BUCKET_FILLED(buckets[i]))
            continue;

        Block* found_block = BLOCK_FROM_BUCKET(world, buckets[i]);
        if (found_block != NULL && found_block->material == M_WIRE)
        {
            if (!found_block->updated || found_block->power < (block->power - 1))
            {
                found_block->power = block->power - 1;
                redstone_wire_update(world, buckets[i]);
            }
        }
        // TODO: Propigate to more materials
    }
}

void redstone_torch_update(World* world, Bucket* bucket)
{
    Block* block = BLOCK_FROM_BUCKET(world, bucket);
    block->updated = 1;

    // TODO: Replace with check for if the torch should turn on or off
    if (block->power < 15)
    {
        block->power = 15;
    }

    Bucket* buckets[5];
    buckets[0] = bucket->adjacent[D_NORTH];
    buckets[1] = bucket->adjacent[D_SOUTH];
    buckets[2] = bucket->adjacent[D_EAST];
    buckets[3] = bucket->adjacent[D_WEST];
    buckets[4] = bucket->adjacent[D_DOWN];

    int i;
    for (i = 0; i < 5; i++)
    {
        if (!BUCKET_FILLED(buckets[i]))
            continue;

        Block* found_block = BLOCK_FROM_BUCKET(world, buckets[i]);
        if (found_block != NULL && found_block->material == M_WIRE)
        {
            found_block->power = block->power;
            redstone_wire_update(world, buckets[i]);
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
                    case M_TORCH:
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
    }

    world->ticks++;
}

