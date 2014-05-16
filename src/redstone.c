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
    do {\
    node->block.last_power = node->block.power;\
    node->block.power = new_power;\
    node->block.updated = true;\
    } while (0)
#define LAST_POWER(node) (node->block.updated ? node->block.last_power : node->block.power)
#define REPEATER_POWERED(node) (node->block.power_state > node->block.state)

static Direction update_power_from_behind(BlockNode* node, int on, int off)
{
    Direction behind = direction_invert(node->block.direction);
    BlockNode* power_source = NODE_ADJACENT(node, behind);
    int new_power = power_source == NULL || LAST_POWER(power_source) == 0 ? off : on;
    UPDATE_POWER(node, new_power);
    return behind;
}

static void redstone_wire_update(World* world, BlockNode* node)
{
    BlockNode* above = NODE_ADJACENT(node, UP);
    bool covered = above != NULL &&
                   above->block.material != AIR &&
                   above->block.material != EMPTY;

    // Adjacent blocks
    Direction directions[4] = {NORTH, SOUTH, EAST, WEST};
    for (int i = 0; i < 4; i++)
    {
        Direction dir = directions[i];

        // Directly adjacent
        BlockNode* found_node = NODE_ADJACENT(node, dir);
        if (found_node == NULL)
            continue;

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
                continue;

            MOVE_TO_NODE(found_node, UP);
        }

        if (found_node == NULL || found_node->block.material != WIRE)
            continue;

        int new_power = node->block.power != 0 ? node->block.power - 1 : 0;
        if SHOULD_UPDATE(found_node, new_power)
        {
            UPDATE_POWER(found_node, new_power);
            redstone_wire_update(world, found_node);
        }
    }

    // Block below
    BlockNode* down_node = NODE_ADJACENT(node, DOWN);
    if (down_node == NULL)
        return;

    if (down_node->block.material == CONDUCTOR && SHOULD_UPDATE(down_node, node->block.power))
    {
        UPDATE_POWER(down_node, node->block.power);
    }
}

static void redstone_comparator_update(World* world, BlockNode* node)
{
    unsigned int side_power = 0;
    BlockNode* right = NODE_ADJACENT(node, direction_right(node->block.direction));
    if (right != NULL && (right->block.material != REPEATER || REPEATER_POWERED(right)))
        side_power = LAST_POWER(right);

    BlockNode* left = NODE_ADJACENT(node, direction_left(node->block.direction));
    if (left != NULL && (left->block.material != REPEATER || REPEATER_POWERED(left)))
    {
        unsigned int left_power = LAST_POWER(left);
        if (left_power > side_power)
        side_power = left_power;
    }

    Direction behind = direction_invert(node->block.direction);
    BlockNode* power_source = NODE_ADJACENT(node, behind);
    int new_power = power_source != NULL ? LAST_POWER(power_source) : 0;
    int change = new_power;

    // Subtraction mode
    if (node->block.state > 0)
        change -= side_power;

    new_power = new_power >= side_power ? change : 0;
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

static void redstone_repeater_update(World* world, BlockNode* node)
{
    // Test if any adjacent repeaters are locking this one
    BlockNode* right = NODE_ADJACENT(node, direction_right(node->block.direction));
    if (right != NULL && right->block.material == REPEATER &&
        LAST_POWER(right) > 0 && REPEATER_POWERED(right))
        return;
    BlockNode* left = NODE_ADJACENT(node, direction_left(node->block.direction));
    if (left != NULL && left->block.material == REPEATER &&
        LAST_POWER(left) > 0 && REPEATER_POWERED(left))
        return;

    update_power_from_behind(node, 15, 0);

    // Update the number of ticks this repeater has been powered
    if (node->block.power != node->block.last_power)
        node->block.power_state = 0;
    if (node->block.power > 0 && !REPEATER_POWERED(node))
        node->block.power_state++;

    // If it's be on shorter than the delay, don't propigate power
    if (!REPEATER_POWERED(node))
        return;

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

static void redstone_torch_update(World* world, BlockNode* node)
{
    Direction behind = update_power_from_behind(node, 0, 15);

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

    MOVE_TO_NODE(up_node, UP);
    if (up_node == NULL || up_node->block.material != WIRE)
        return;

    UPDATE_POWER(up_node, node->block.power);
    redstone_wire_update(world, up_node);
}

void redstone_tick(World* world, void (*block_modified_callback)(Block*))
{
    // Process all power sources
    for (BlockNode* node = world->blocks->head; node != NULL; node = node->next)
    {
        switch (node->block.material)
        {
            case TORCH:      redstone_torch_update(world, node);      break;
            case REPEATER:   redstone_repeater_update(world, node);   break;
            case COMPARATOR: redstone_comparator_update(world, node); break;
            default:         goto end;
        }

        node->block.updated = true;
    }
end:

    // Check for block modifications and reset flags
    for (BlockNode* node = world->blocks->tail; node != NULL; node = node->prev)
    {
        if (node->block.updated)
        {
            if (node->block.power != node->block.last_power)
                block_modified_callback(&node->block);
            node->block.updated = false;
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

