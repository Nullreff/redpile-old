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
#include <assert.h>
#include "world.h"
#include "block.h"
#include "rup.h"
#include "redstone.h"

#define MAX_POWER 15
#define MOVE_TO_NODE(node,dir) node = NODE_ADJACENT(node, dir)
#define SHOULD_UPDATE(node,new_power) (rup_get_power(rup, &(node)->block) < (new_power))
#define UPDATE_POWER(node,new_power) rup_cmd_power(rup, &(node)->block, new_power)
#define LAST_POWER(node) (node)->block.power
#define REPEATER_POWERED(node) (node->block.power_state > node->block.state)
#define MATERIAL_IS(node,name) (node->block.material == name)
#define MATERIAL_ISNT(node,name) (node->block.material != name)
#define NODE_ADJACENT(node,dir) world_get_adjacent_block(world, node, dir)

static bool can_power_from_behind(BlockNode* node, Direction dir)
{
    switch (node->block.material)
    {
        // Back must be attached to this block
        case TORCH:
        case REPEATER:
        case COMPARATOR:
            return node->block.direction == dir;

        // Should not be facing towards this block
        case PISTON:
            return node->block.direction != direction_invert(dir);

        default:
            return false;
    }
}

static void redstone_conductor_update(Rup* rup, World* world, BlockNode* node)
{
    assert(MATERIAL_IS(node, CONDUCTOR));

    for (int i = 0; i < DIRECTIONS_COUNT; i++)
    {
        Direction dir = (Direction)i;
        BlockNode* found_node = NODE_ADJACENT(node, dir);
        unsigned int new_power = rup_get_power(rup, &node->block);
        if (can_power_from_behind(found_node, dir) &&
            SHOULD_UPDATE(found_node, new_power))
        {
            UPDATE_POWER(found_node, new_power);
        }
    }
}

static void redstone_wire_update(Rup* rup, World* world, BlockNode* node)
{
    assert(MATERIAL_IS(node, WIRE));

    BlockNode* above = NODE_ADJACENT(node, UP);
    bool covered = above != NULL && MATERIAL_ISNT(above, AIR) && MATERIAL_ISNT(above, EMPTY);

    int new_power = rup_get_power(rup, &node->block);
    int wire_power = new_power;
    if (wire_power > 0)
        wire_power--;

    // Adjacent blocks
    Direction directions[4] = {NORTH, SOUTH, EAST, WEST};
    for (int i = 0; i < 4; i++)
    {
        Direction dir = directions[i];

        // Directly adjacent
        BlockNode* found_node = NODE_ADJACENT(node, dir);

        // Down one block
        if (MATERIAL_IS(found_node, AIR))
        {
            MOVE_TO_NODE(found_node, DOWN);
            if (MATERIAL_ISNT(found_node, WIRE))
                continue;
        }
        // Up one block
        else if (MATERIAL_IS(found_node, CONDUCTOR))
        {
            // Add charge to the block
            BlockNode* right = NODE_ADJACENT(node, direction_left(dir));
            BlockNode* left = NODE_ADJACENT(node, direction_right(dir));

            if (MATERIAL_ISNT(right, WIRE) && MATERIAL_ISNT(left, WIRE) &&
                SHOULD_UPDATE(found_node, new_power))
            {
                UPDATE_POWER(found_node, new_power);
                redstone_conductor_update(rup, world, found_node);
            }

            if (covered)
                continue;

            MOVE_TO_NODE(found_node, UP);
            if (MATERIAL_ISNT(found_node, WIRE))
                continue;
        }
        // Block in front
        else if (MATERIAL_ISNT(found_node, WIRE) &&
                !can_power_from_behind(found_node, dir))
        {
            continue;
        }

        if SHOULD_UPDATE(found_node, wire_power)
        {
            UPDATE_POWER(found_node, wire_power);
            if (MATERIAL_IS(found_node, WIRE))
                redstone_wire_update(rup, world, found_node);
            else if (MATERIAL_IS(found_node, CONDUCTOR))
                redstone_conductor_update(rup, world, found_node);
        }
    }

    // Block below
    BlockNode* down_node = NODE_ADJACENT(node, DOWN);
    if (MATERIAL_IS(down_node, CONDUCTOR) && SHOULD_UPDATE(down_node, new_power))
    {
        UPDATE_POWER(down_node, new_power);
        redstone_conductor_update(rup, world, down_node);
    }
}

static void redstone_piston_update(Rup* rup, World* world, BlockNode* node)
{
    assert(MATERIAL_IS(node, PISTON));

    BlockNode* first = NODE_ADJACENT(node, node->block.direction);
    BlockNode* second = NODE_ADJACENT(first, node->block.direction);

    if (LAST_POWER(node) > 0)
    {
        if (MATERIAL_ISNT(first, INSULATOR))
        {
            rup_cmd_move(rup, &first->block, &second->block);
            rup_cmd_set(rup, &first->block, INSULATOR);
        }
    }
    else
    {
        if (MATERIAL_IS(first, INSULATOR))
        {
            rup_cmd_move(rup, &second->block, &first->block);
            rup_cmd_set(rup, &second->block, AIR);
        }
    }
}

static void redstone_comparator_update(Rup* rup, World* world, BlockNode* node)
{
    assert(MATERIAL_IS(node, COMPARATOR));

    if (LAST_POWER(node) == 0)
        return;

    unsigned int side_power = 0;
    BlockNode* right = NODE_ADJACENT(node, direction_right(node->block.direction));
    if (MATERIAL_ISNT(right, REPEATER) || REPEATER_POWERED(right))
    {
        side_power = MATERIAL_IS(right, TORCH) ? MAX_POWER : LAST_POWER(right);
    }

    BlockNode* left = NODE_ADJACENT(node, direction_left(node->block.direction));
    if (MATERIAL_ISNT(left, REPEATER) || REPEATER_POWERED(left))
    {
        unsigned int left_power = MATERIAL_IS(left, TORCH) ? MAX_POWER : LAST_POWER(left);
        if (left_power > side_power)
            side_power = left_power;
    }

    int new_power = LAST_POWER(node);
    int change = new_power;

    // Subtraction mode
    if (node->block.state > 0)
        change -= side_power;

    new_power = new_power > side_power ? change : 0;
    UPDATE_POWER(node, new_power);

    if (new_power == 0)
        return;

    // Pass charge to the wire or conductor in front
    BlockNode* found_node = NODE_ADJACENT(node, node->block.direction);
    if (MATERIAL_IS(found_node, WIRE))
    {
        UPDATE_POWER(found_node, MAX_POWER);
        redstone_wire_update(rup, world, found_node);
    }
    else if (MATERIAL_IS(found_node, CONDUCTOR))
    {
        UPDATE_POWER(found_node, MAX_POWER);
        redstone_conductor_update(rup, world, found_node);
    }
}

static void redstone_repeater_update(Rup* rup, World* world, BlockNode* node)
{
    assert(MATERIAL_IS(node, REPEATER));

    if (LAST_POWER(node) == 0)
    {
        node->block.power_state = 0;
        return;
    }

    // Test if any adjacent repeaters are locking this one
    BlockNode* right = NODE_ADJACENT(node, direction_right(node->block.direction));
    if (MATERIAL_IS(right, REPEATER) && LAST_POWER(right) > 0 && REPEATER_POWERED(right))
        return;

    BlockNode* left = NODE_ADJACENT(node, direction_left(node->block.direction));
    if (MATERIAL_IS(left, REPEATER) && LAST_POWER(left) > 0 && REPEATER_POWERED(left))
        return;

    // Update the number of ticks this repeater has been powered
    unsigned int new_power = rup_get_power(rup, &node->block);
    unsigned int new_power_state = node->block.power_state;
    if (new_power > 0 && !REPEATER_POWERED(node))
    {
        new_power_state++;
        rup_cmd_state(rup, &node->block, new_power_state);
    }

    // If it's be on shorter than the delay, don't propigate power
    if (new_power_state <= node->block.state)
        return;

    // Pass charge to the wire or conductor in front
    BlockNode* found_node = NODE_ADJACENT(node, node->block.direction);
    if (MATERIAL_IS(found_node, WIRE))
    {
        UPDATE_POWER(found_node, MAX_POWER);
        redstone_wire_update(rup, world, found_node);
    }
    else if (MATERIAL_IS(found_node, CONDUCTOR))
    {
        UPDATE_POWER(found_node, MAX_POWER);
        redstone_conductor_update(rup, world, found_node);
    }
}

static void redstone_torch_update(Rup* rup, World* world, BlockNode* node)
{
    assert(MATERIAL_IS(node, TORCH));

    if (LAST_POWER(node) > 0)
        return;

    // Pass charge to any adjacent wires
    Direction behind = direction_invert(node->block.direction);
    Direction directions[5] = {NORTH, SOUTH, EAST, WEST, DOWN};
    for (int i = 0; i < 5; i++)
    {
        Direction dir = directions[i];
        if (dir == behind)
            continue;

        BlockNode* found_node = NODE_ADJACENT(node, dir);
        if (can_power_from_behind(found_node, dir) ||
            MATERIAL_IS(found_node, WIRE))
        {
            UPDATE_POWER(found_node, MAX_POWER);
            if (MATERIAL_IS(found_node, WIRE))
                redstone_wire_update(rup, world, found_node);
        }
    }

    // Pass charge up through a block
    BlockNode* up_node = NODE_ADJACENT(node, UP);
    if (MATERIAL_ISNT(up_node, CONDUCTOR))
        return;

    UPDATE_POWER(up_node, MAX_POWER);

    MOVE_TO_NODE(up_node, UP);
    if (MATERIAL_ISNT(up_node, WIRE))
        return;

    UPDATE_POWER(up_node, MAX_POWER);
    redstone_wire_update(rup, world, up_node);
}

static bool redstone_block_missing(Block* block)
{
    block->material = AIR;
    return true;
}

void redstone_tick(World* world, void (*block_modified_callback)(Block*))
{
    RupList* rup_list = rup_list_allocate(world->blocks->power_sources);
    world_set_block_missing_callback(world, redstone_block_missing);

    // Process all power sources
    int index = 0;
    for (BlockNode* node = world->blocks->head; index < rup_list->size && node != NULL; node = node->next, index++)
    {
        Rup* rup = rup_list->rups[index];
        switch (node->block.material)
        {
            case TORCH:      redstone_torch_update(rup, world, node);      break;
            case REPEATER:   redstone_repeater_update(rup, world, node);   break;
            case COMPARATOR: redstone_comparator_update(rup, world, node); break;
            case PISTON:     redstone_piston_update(rup, world, node);     break;
            default: ERROR("Encountered non power source in the start of the block list");
        }
    }

    Runmap* runmap = runmap_allocate(world->blocks->size);
    for (int i = 0; i < rup_list->size; i++)
    {
        runmap_import(runmap, rup_list->rups[i]);
    }


    for (int i = 0; i < runmap->size; i++)
    for (Bucket* bucket = runmap->data + i; BUCKET_FILLED(bucket); bucket = bucket->next)
    for (RupInst* inst = bucket->value; inst != NULL; inst = inst->next)
    {
        rup_run(inst);
    }

    // Check for block modifications and reset flags
    for (BlockNode* node = world->blocks->tail; node != NULL; node = node->prev)
    {
        if (runmap_block_power_changed(runmap, &node->block))
        {
            if (node->block.updated)
            {
                block_modified_callback(&node->block);
                node->block.updated = false;
            }
        }
        else if (node->block.power > 0)
        {
            // Blocks not connected to a power source become unpowered
            node->block.power = 0;
            block_modified_callback(&node->block);
        }
    }

    world->ticks++;
    world_clear_block_missing_callback(world);
    runmap_free(runmap);
    rup_list_free(rup_list);
}

