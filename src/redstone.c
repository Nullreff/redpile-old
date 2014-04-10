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

void (*redstone_propigate[6])(World*, Block*) = {
    redstone_noop_update,
    redstone_noop_update,
    redstone_wire_update,
    redstone_conductor_update,
    redstone_noop_update,
    redstone_torch_update
};

void redstone_propigate_power(World* world, Block* block)
{
    redstone_propigate[block->material](world, block);
}

void redstone_noop_update(World* world, Block* block)
{
    block->updated = 1;
}

void redstone_wire_update(World* world, Block* block)
{
    block->updated = 1;

    Block* blocks[4];
    blocks[0] = world_get_block(world, location_move(block->location, D_NORTH, 1));
    blocks[1] = world_get_block(world, location_move(block->location, D_SOUTH, 1));
    blocks[2] = world_get_block(world, location_move(block->location, D_EAST, 1));
    blocks[3] = world_get_block(world, location_move(block->location, D_WEST, 1));

    int i;
    for (i = 0; i < 4; i++)
    {
        if (blocks[i] != NULL && blocks[i]->material == M_WIRE)
        {
            if (!blocks[i]->updated || blocks[i]->power < (block->power - 1))
            {
                blocks[i]->power = block->power - 1;
                redstone_wire_update(world, blocks[i]);
            }
        }
        // TODO: Propigate to more materials
    }
}

void redstone_conductor_update(World* world, Block* block)
{
    block->updated = 1;
    // TODO: Propigate power
}

void redstone_torch_update(World* world, Block* block)
{
    block->updated = 1;

    // TODO: Replace with check for if the torch should turn on or off
    if (block->power < 15)
    {
        block->power = 15;
    }

    Block* blocks[5];
    blocks[0] = world_get_block(world, location_move(block->location, D_NORTH, 1));
    blocks[1] = world_get_block(world, location_move(block->location, D_SOUTH, 1));
    blocks[2] = world_get_block(world, location_move(block->location, D_EAST, 1));
    blocks[3] = world_get_block(world, location_move(block->location, D_WEST, 1));
    blocks[4] = world_get_block(world, location_move(block->location, D_DOWN, 1));

    int i;
    for (i = 0; i < 5; i++)
    {
        if (blocks[i] != NULL && blocks[i]->material == M_WIRE)
        {
            blocks[i]->power = block->power;
            redstone_wire_update(world, blocks[i]);
        }
        // TODO: Propigate to more materials
    }
}

void redstone_tick(World* world, void (*block_modified_callback)(Block*))
{
    int i;

    // Process all power sources
    for (i = 0; i < world->blocks_size; i++)
    {
        Block* block = world->blocks + i;

        // Skip any blocks that have already been processed
        if (block->updated)
            continue;

        switch (block->material)
        {
            // Add more powers sources here as needed
            case M_TORCH:
                redstone_torch_update(world, block);
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

