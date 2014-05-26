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
#include "redstone.h"

#define MAX_POWER 15
#define MOVE_TO_NODE(node,dir) node = NODE_ADJACENT(node, dir)
#define SHOULD_UPDATE(NODE,POWER,SOURCE)\
    ((NODE)->new_power == UINT_MAX ||\
     (location_equals((NODE)->power_source, SOURCE) && (NODE)->new_power < POWER))
#define UPDATE_POWER(NODE,POWER,SOURCE) do {\
   (NODE)->new_power = POWER;\
   (NODE)->power_source = SOURCE;\
   rup_cmd_power(rup, &(NODE)->block, POWER);\
} while (0)
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

static void redstone_conductor_update(Rup* rup, World* world, BlockNode* node, unsigned int new_power, Location power_source)
{
    assert(MATERIAL_IS(node, CONDUCTOR));

    for (int i = 0; i < DIRECTIONS_COUNT; i++)
    {
        Direction dir = (Direction)i;
        BlockNode* found_node = NODE_ADJACENT(node, dir);
        if (can_power_from_behind(found_node, dir) &&
            SHOULD_UPDATE(found_node, new_power, power_source))
        {
            UPDATE_POWER(found_node, new_power, power_source);
        }
    }
}

static void redstone_wire_update(Rup* rup, World* world, BlockNode* node, unsigned int new_power, Location power_source)
{
    assert(MATERIAL_IS(node, WIRE));

    BlockNode* above = NODE_ADJACENT(node, UP);
    bool covered = above != NULL && MATERIAL_ISNT(above, AIR) && MATERIAL_ISNT(above, EMPTY);

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
                SHOULD_UPDATE(found_node, new_power, power_source))
            {
                UPDATE_POWER(found_node, new_power, power_source);
                redstone_conductor_update(rup, world, found_node, new_power, power_source);
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

        if SHOULD_UPDATE(found_node, wire_power, power_source)
        {
            UPDATE_POWER(found_node, wire_power, power_source);
            if (MATERIAL_IS(found_node, WIRE))
                redstone_wire_update(rup, world, found_node, wire_power, power_source);
            else if (MATERIAL_IS(found_node, CONDUCTOR))
                redstone_conductor_update(rup, world, found_node, wire_power, power_source);
        }
    }

    // Block below
    BlockNode* down_node = NODE_ADJACENT(node, DOWN);
    if (MATERIAL_IS(down_node, CONDUCTOR) && SHOULD_UPDATE(down_node, new_power, power_source))
    {
        UPDATE_POWER(down_node, new_power, power_source);
        redstone_conductor_update(rup, world, down_node, new_power, power_source);
    }
}

static void redstone_piston_update(Rup* rup, World* world, BlockNode* node)
{
    assert(MATERIAL_IS(node, PISTON));

    BlockNode* first = NODE_ADJACENT(node, node->block.direction);
    BlockNode* second = NODE_ADJACENT(first, node->block.direction);

    if (LAST_POWER(node) > 0)
    {
        if (MATERIAL_IS(second, AIR))
            rup_cmd_swap(rup, &first->block, &second->block);
    }
    else
    {
        if (MATERIAL_IS(first, AIR))
            rup_cmd_swap(rup, &first->block, &second->block);
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

    if (new_power == 0)
        return;

    // Pass charge to the wire or conductor in front
    BlockNode* found_node = NODE_ADJACENT(node, node->block.direction);
    if (MATERIAL_IS(found_node, WIRE))
    {
        UPDATE_POWER(found_node, new_power, node->block.location);
        redstone_wire_update(rup, world, found_node, new_power, node->block.location);
    }
    else if (MATERIAL_IS(found_node, CONDUCTOR))
    {
        UPDATE_POWER(found_node, new_power, node->block.location);
        redstone_conductor_update(rup, world, found_node, new_power, node->block.location);
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
    unsigned int new_power_state = node->block.power_state;
    if (node->block.power > 0 && !REPEATER_POWERED(node))
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
        UPDATE_POWER(found_node, MAX_POWER, node->block.location);
        redstone_wire_update(rup, world, found_node, MAX_POWER, node->block.location);
    }
    else if (MATERIAL_IS(found_node, CONDUCTOR))
    {
        UPDATE_POWER(found_node, MAX_POWER, node->block.location);
        redstone_conductor_update(rup, world, found_node, MAX_POWER, node->block.location);
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
            UPDATE_POWER(found_node, MAX_POWER, node->block.location);
            if (MATERIAL_IS(found_node, WIRE))
                redstone_wire_update(rup, world, found_node, MAX_POWER, node->block.location);
        }
    }

    // Pass charge up through a block
    BlockNode* up_node = NODE_ADJACENT(node, UP);
    if (MATERIAL_ISNT(up_node, CONDUCTOR))
        return;

    UPDATE_POWER(up_node, MAX_POWER, node->block.location);

    MOVE_TO_NODE(up_node, UP);
    if (MATERIAL_ISNT(up_node, WIRE))
        return;

    UPDATE_POWER(up_node, MAX_POWER, node->block.location);
    redstone_wire_update(rup, world, up_node, MAX_POWER, node->block.location);
}

static bool redstone_block_missing(Block* block)
{
    block->material = AIR;
    return true;
}

void redstone_tick(World* world, void (*rup_inst_run_callback)(RupInst*))
{
    RupList* rup_list = rup_list_allocate(world->blocks->sizes[BOUNDARY]);
    world_set_block_missing_callback(world, redstone_block_missing);

    // Process all power sources
    int index = 0;
    FOR_LIST(BlockNode, node, world->blocks->nodes[BOUNDARY])
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
        index++;
    }

    // Aggregate and sort update commands
    Runmap* runmap = runmap_allocate();
    for (int i = 0; i < rup_list->size; i++)
    {
        runmap_import(runmap, rup_list->rups[i]);
    }

    // Run update commands
    for (int i = 0; i < RUP_CMD_COUNT; i++)
    for (RupInst* inst = runmap->instructions[i]; inst != NULL; inst = inst->next)
    {
        rup_inst_run_callback(inst);
        rup_inst_run(world, inst);
    }

    // Check for unpowered blocks and reset flags
#define UNPOWER_CODE(TYPE)\
    FOR_LIST(BlockNode, node, world->blocks->nodes[TYPE])\
    {\
        if (node->block.updated)\
        {\
            node->block.updated = false;\
        }\
        else if (node->block.power > 0)\
        {\
            RupInst inst = rup_inst_create(RUP_POWER, &node->block);\
            inst.value.power = 0;\
            rup_inst_run_callback(&inst);\
            rup_inst_run(world, &inst);\
        }\
    }

    UNPOWER_CODE(BOUNDARY)
    UNPOWER_CODE(POWERABLE)

    world->ticks++;
    world_clear_block_missing_callback(world);
    runmap_free(runmap);
    rup_list_free(rup_list);
}

