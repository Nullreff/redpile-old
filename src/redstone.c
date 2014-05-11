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

#define MOVE_TO_NODE(node,dir) node = NODE_ADJACENT(node, dir)
#define SHOULD_UPDATE(node,new_power) (!(node)->block.updated || (node)->block.power < (new_power))
#define UPDATE_POWER(node,new_power)\
    node->block.last_power = node->block.power;\
    node->block.power = new_power;\
    node->block.updated = 1
#define LAST_POWER(node) (node->block.updated ? node->block.last_power : node->block.power)

void redstone_wire_update(World* world, BlockNode* node);

void redstone_wire_update_adjacent(World* world, BlockNode* node, Direction dir, bool covered)
{
    // Directly adjacent
    BlockNode* found_node = NODE_ADJACENT(node, dir);
    if (found_node == NULL)
        return;

    // Down one block
    if (found_node->block.material == EMPTY || found_node->block.material == AIR)
    {
        MOVE_TO_NODE(found_node, DOWN);
    }
    // Up one block
    else if (found_node->block.material == CONDUCTOR)
    {
        // Add charge to the block
        BlockNode* right = NODE_ADJACENT(node, direction_left(dir));
        BlockNode* left = NODE_ADJACENT(node, direction_right(dir));

        if ((right == NULL || right->block.material != WIRE) &&
            (left == NULL || left->block.material != WIRE) &&
            SHOULD_UPDATE(found_node, node->block.power))
        {
            UPDATE_POWER(found_node, node->block.power);
        }

        if (covered)
            return;

        MOVE_TO_NODE(found_node, UP);
    }

    if (found_node == NULL || found_node->block.material != WIRE)
        return;

    int new_power = node->block.power - 1;
    if SHOULD_UPDATE(found_node, new_power)
    {
        UPDATE_POWER(found_node, new_power);
        redstone_wire_update(world, found_node);
    }
}

void redstone_wire_update(World* world, BlockNode* node)
{
    BlockNode* above = NODE_ADJACENT(node, UP);
    bool covered = above != NULL &&
                   above->block.material != AIR &&
                   above->block.material != EMPTY;

    // Adjacent blocks
    Direction directions[4] = {NORTH, SOUTH, EAST, WEST};
    for (int i = 0; i < 4; i++)
        redstone_wire_update_adjacent(world, node, directions[i], covered);

    // Block below
    BlockNode* down_node = NODE_ADJACENT(node, DOWN);
    if (down_node == NULL)
        return;

    if (down_node->block.material == CONDUCTOR && SHOULD_UPDATE(down_node, node->block.power))
    {
        UPDATE_POWER(down_node, node->block.power);
    }
}

void redstone_repeater_update(World* world, BlockNode* node)
{
    // Set the repeater power from the block behind it
    Direction behind = direction_invert(node->block.direction);
    BlockNode* power_source = NODE_ADJACENT(node, behind);
    int new_power = power_source == NULL || LAST_POWER(power_source) == 0 ? 0 : 15;
    UPDATE_POWER(node, new_power);

    // Pass charge to the wire or conductor in front
    BlockNode* found_node = NODE_ADJACENT(node, node->block.direction);
    if (found_node == NULL)
        return;

    if (found_node->block.material == WIRE)
    {
        UPDATE_POWER(found_node, node->block.power);
        redstone_wire_update(world, found_node);
    }
    else if (found_node->block.material == CONDUCTOR)
    {
        UPDATE_POWER(found_node, node->block.power);
    }
}

void redstone_torch_update(World* world, BlockNode* node)
{
    // Set the torch power from the block behind it
    Direction behind = direction_invert(node->block.direction);
    BlockNode* power_source = NODE_ADJACENT(node, behind);
    int new_power = power_source == NULL || LAST_POWER(power_source) == 0 ? 15 : 0;
    UPDATE_POWER(node, new_power);

    // Pass charge to any adjacent wires
    Direction directions[5] = {NORTH, SOUTH, EAST, WEST, DOWN};
    for (int i = 0; i < 5; i++)
    {
        if (directions[i] == behind)
            continue;

        BlockNode* found_node = NODE_ADJACENT(node, directions[i]);
        if (found_node == NULL)
            continue;

        if (found_node->block.material == WIRE)
        {
            UPDATE_POWER(found_node, node->block.power);
            redstone_wire_update(world, found_node);
        }
    }

    // Pass charge up through a block
    BlockNode* up_node = NODE_ADJACENT(node, UP);
    if (up_node == NULL || up_node->block.material != CONDUCTOR)
        return;

    UPDATE_POWER(up_node, node->block.power);

    BlockNode* up_2_node = NODE_ADJACENT(up_node, UP);
    if (up_2_node == NULL || up_2_node->block.material != WIRE)
        return;

    UPDATE_POWER(up_2_node, node->block.power);
    redstone_wire_update(world, up_2_node);
}

void redstone_tick(World* world, void (*block_modified_callback)(Block*))
{
    // Process all power sources
    for (BlockNode* node = world->blocks->head; node != NULL; node = node->next)
    {
        switch (node->block.material)
        {
            case TORCH:
                redstone_torch_update(world, node);
                break;
            case REPEATER:
                redstone_repeater_update(world, node);
                break;
            default:
                goto end;
        }

        node->block.updated = 1;
    }
end:

    // Check for block modifications and reset flags
    for (BlockNode* node = world->blocks->tail; node != NULL; node = node->prev)
    {
        if (node->block.updated)
        {
            block_modified_callback(&node->block);
            node->block.updated = 0;
        }
        else if (node->block.power > 0)
        {
            // Blocks not connected to a power source become unpowered
            node->block.power = 0;
            block_modified_callback(&node->block);
        }
    }

    world->ticks++;
}

