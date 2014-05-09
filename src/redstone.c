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

#define MOVE_TO_BLOCK(block,dir) block = BLOCK_ADJACENT(world, block, dir)
#define SHOULD_UPDATE(block,new_power) (!(block)->updated || (block)->power < (new_power))

void redstone_wire_update(World* world, Block* block);

void redstone_wire_update_adjacent(World* world, Block* block, Direction dir, bool covered)
{
    // Directly adjacent
    Block* found_block = BLOCK_ADJACENT(world, block, dir);
    if (found_block == NULL)
        return;

    // Down one block
    if (found_block->material == EMPTY || found_block->material == AIR)
    {
        MOVE_TO_BLOCK(found_block, DOWN);
    }
    // Up one block
    else if (found_block->material == CONDUCTOR)
    {
        // Add charge to the block
        Block* right = BLOCK_ADJACENT(world, block, direction_left(dir));
        Block* left = BLOCK_ADJACENT(world, block, direction_right(dir));

        if ((right == NULL || right->material != WIRE) &&
            (left == NULL || left->material != WIRE) &&
            SHOULD_UPDATE(found_block, block->power))
        {
            world_set_last_power(world, found_block);
            found_block->power = block->power;
            found_block->updated = 1;
        }

        if (covered)
            return;

        MOVE_TO_BLOCK(found_block, UP);
    }

    if (found_block == NULL || found_block->material != WIRE)
        return;

    int new_power = block->power - 1;
    if SHOULD_UPDATE(found_block, new_power)
    {
        world_set_last_power(world, found_block);
        found_block->power = new_power;
        redstone_wire_update(world, found_block);
    }
}

void redstone_wire_update(World* world, Block* block)
{
    block->updated = 1;

    Block* above = BLOCK_ADJACENT(world, block, UP);
    bool covered = above != NULL &&
                   above->material != AIR &&
                   above->material != EMPTY;

    // Adjacent blocks
    Direction directions[4] = {NORTH, SOUTH, EAST, WEST};
    for (int i = 0; i < 4; i++)
        redstone_wire_update_adjacent(world, block, directions[i], covered);

    // Block below
    Block* down_block = BLOCK_ADJACENT(world, block, DOWN);
    if (down_block == NULL)
        return;

    if (down_block->material == CONDUCTOR && SHOULD_UPDATE(down_block, block->power))
    {
        world_set_last_power(world, down_block);
        down_block->power = block->power;
        down_block->updated = 1;
    }
}

void redstone_repeater_update(World* world, Block* block)
{
    block->updated = 1;

    // Set the repeater power from the block behind it
    Direction behind = direction_invert(block->direction);
    Block* power_source = BLOCK_ADJACENT(world, block, behind);
    if (power_source != NULL)
    {
        int power = world_get_last_power(world, power_source);
        block->power = power == 0 ? 0 : 15;
    }
    else
    {
        block->power = 0;
    }

    // Pass charge to the wire or conductor in front
    Block* found_block = BLOCK_ADJACENT(world, block, block->direction);
    if (found_block == NULL)
        return;

    if (found_block->material == WIRE)
    {
        world_set_last_power(world, found_block);
        found_block->power = block->power;
        redstone_wire_update(world, found_block);
    }
    else if (found_block->material == CONDUCTOR)
    {
        world_set_last_power(world, found_block);
        found_block->power = block->power;
        found_block->updated = 1;
    }
}

void redstone_torch_update(World* world, Block* block)
{
    block->updated = 1;

    // Set the torch power from the block behind it
    Direction behind = direction_invert(block->direction);
    Block* power_source = BLOCK_ADJACENT(world, block, behind);
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

        Block* found_block = BLOCK_ADJACENT(world, block, directions[i]);
        if (found_block == NULL)
            continue;

        if (found_block->material == WIRE)
        {
            world_set_last_power(world, found_block);
            found_block->power = block->power;
            redstone_wire_update(world, found_block);
        }
    }

    // Pass charge up through a block
    Block* up_block = BLOCK_ADJACENT(world, block, UP);
    if (up_block == NULL || up_block->material != CONDUCTOR)
        return;

    world_set_last_power(world, up_block);
    up_block->power = block->power;
    up_block->updated = 1;

    Block* up_2_block = BLOCK_ADJACENT(world, up_block, UP);
    if (up_2_block == NULL || up_2_block->material != WIRE)
        return;

    world_set_last_power(world, up_2_block);
    up_2_block->power = block->power;
    redstone_wire_update(world, up_2_block);
}

void redstone_tick(World* world, void (*block_modified_callback)(Block*))
{
    // Process all power sources
    for (int i = 0; i < world->blocks->index; i++)
    {
        Block* block = INDEX_BLOCK(world, i);
        if (block == NULL || block->updated)
            continue;

        switch (block->material)
        {
            case TORCH:    redstone_torch_update(world, block);    break;
            case REPEATER: redstone_repeater_update(world, block); break;
        }
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
        world_reset_last_power(world, i);
    }

    world->ticks++;
}

