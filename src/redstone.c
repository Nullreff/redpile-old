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
#define UPDATE_POWER(NODE,POWER)\
if (!(NODE)->block.powered) {\
    (NODE)->block.powered = true;\
    rup_cmd_power(rup, NODE, POWER);\
}
#define POWER(node) (node)->block.power
#define REPEATER_POWERED(node) (node->block.power_state > node->block.state)
#define MATERIAL_IS(node,name) (node->block.material == name)
#define MATERIAL_ISNT(node,name) (node->block.material != name)
#define NODE_ADJACENT(node,dir) world_get_adjacent_block(world, node, dir)

static bool can_power_from_behind(BlockNode* node, Direction dir)
{
    if (!M_BOUNDARY(node->block.material))
        return false;

    // Pistons should not be facing towards this block
    if (node->block.material == PISTON)
        return node->block.direction != direction_invert(dir);

    // Otherwise, the back face must be attached to this block
    return node->block.direction == dir;
}

static void redstone_conductor_update(Rup* rup, World* world, BlockNode* node, unsigned int new_power)
{
    assert(MATERIAL_IS(node, CONDUCTOR));

    for (int i = 0; i < DIRECTIONS_COUNT; i++)
    {
        Direction dir = (Direction)i;
        BlockNode* found_node = NODE_ADJACENT(node, dir);
        if (can_power_from_behind(found_node, dir))
            UPDATE_POWER(found_node, new_power);
    }
}

static void redstone_wire_update(Rup* rup, World* world, BlockNode* node, unsigned int new_power)
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

            if (MATERIAL_ISNT(right, WIRE) && MATERIAL_ISNT(left, WIRE))
                UPDATE_POWER(found_node, new_power);

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

        UPDATE_POWER(found_node, wire_power);
    }

    // Block below
    BlockNode* down_node = NODE_ADJACENT(node, DOWN);
    if (MATERIAL_IS(down_node, CONDUCTOR))
        UPDATE_POWER(down_node, new_power);
}

static void redstone_piston_update(Rup* rup, World* world, BlockNode* node)
{
    assert(MATERIAL_IS(node, PISTON));

    BlockNode* first = NODE_ADJACENT(node, node->block.direction);
    BlockNode* second = NODE_ADJACENT(first, node->block.direction);

    if (POWER(node) > 0)
    {
        if (MATERIAL_IS(second, AIR))
            rup_cmd_swap(rup, first, second);
    }
    else
    {
        if (MATERIAL_IS(first, AIR))
            rup_cmd_swap(rup, first, second);
    }
}

static void redstone_comparator_update(Rup* rup, World* world, BlockNode* node)
{
    assert(MATERIAL_IS(node, COMPARATOR));

    if (POWER(node) == 0)
        return;

    unsigned int side_power = 0;
    BlockNode* right = NODE_ADJACENT(node, direction_right(node->block.direction));
    if (MATERIAL_ISNT(right, REPEATER) || REPEATER_POWERED(right))
    {
        side_power = MATERIAL_IS(right, TORCH) ? MAX_POWER : POWER(right);
    }

    BlockNode* left = NODE_ADJACENT(node, direction_left(node->block.direction));
    if (MATERIAL_ISNT(left, REPEATER) || REPEATER_POWERED(left))
    {
        unsigned int left_power = MATERIAL_IS(left, TORCH) ? MAX_POWER : POWER(left);
        if (left_power > side_power)
            side_power = left_power;
    }

    int new_power = POWER(node);
    int change = new_power;

    // Subtraction mode
    if (node->block.state > 0)
        change -= side_power;

    new_power = new_power > side_power ? change : 0;

    if (new_power == 0)
        return;

    // Pass charge to the wire or conductor in front
    BlockNode* found_node = NODE_ADJACENT(node, node->block.direction);
    if (MATERIAL_IS(found_node, WIRE) || MATERIAL_IS(found_node, CONDUCTOR))
        UPDATE_POWER(found_node, new_power);
}

static void redstone_repeater_update(Rup* rup, World* world, BlockNode* node)
{
    assert(MATERIAL_IS(node, REPEATER));

    if (POWER(node) == 0)
    {
        node->block.power_state = 0;
        return;
    }

    // Test if any adjacent repeaters are locking this one
    BlockNode* right = NODE_ADJACENT(node, direction_right(node->block.direction));
    if (MATERIAL_IS(right, REPEATER) && POWER(right) > 0 && REPEATER_POWERED(right))
        return;

    BlockNode* left = NODE_ADJACENT(node, direction_left(node->block.direction));
    if (MATERIAL_IS(left, REPEATER) && POWER(left) > 0 && REPEATER_POWERED(left))
        return;

    // Update the number of ticks this repeater has been powered
    unsigned int new_power_state = node->block.power_state;
    if (node->block.power > 0 && !REPEATER_POWERED(node))
    {
        new_power_state++;
        rup_cmd_state(rup, node, new_power_state);
    }

    // If it's be on shorter than the delay, don't propigate power
    if (new_power_state <= node->block.state)
        return;

    // Pass charge to the wire or conductor in front
    BlockNode* found_node = NODE_ADJACENT(node, node->block.direction);
    if (MATERIAL_IS(found_node, WIRE) || MATERIAL_IS(found_node, CONDUCTOR))
        UPDATE_POWER(found_node, MAX_POWER);
}

static void redstone_torch_update(Rup* rup, World* world, BlockNode* node)
{
    assert(MATERIAL_IS(node, TORCH));

    if (POWER(node) > 0)
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
        if (can_power_from_behind(found_node, dir) || MATERIAL_IS(found_node, WIRE))
            UPDATE_POWER(found_node, MAX_POWER);
    }

    // Pass charge up through a block
    BlockNode* up_node = NODE_ADJACENT(node, UP);
    if (MATERIAL_IS(up_node, CONDUCTOR))
    {
        UPDATE_POWER(up_node, MAX_POWER);
        MOVE_TO_NODE(up_node, UP);
        if (MATERIAL_IS(up_node, WIRE))
            UPDATE_POWER(up_node, MAX_POWER);
    }
}

static bool redstone_block_missing(Block* block)
{
    block->material = AIR;
    return true;
}

static void redstone_find_unpowered(World* world, BlockType type, void (*rup_inst_run_callback)(RupInst*))
{
    BlockNode* node;
    FOR_BLOCK_LIST(node, world->blocks, type)
    {
        if (node->block.powered)
        {
            node->block.powered = false;
        }
        else if (node->block.power > 0)
        {
            RupInst inst = rup_inst_create(RUP_POWER, node);
            inst.value.power = 0;
            rup_inst_run_callback(&inst);
            rup_inst_run(world, &inst);
        }
    }
}

void redstone_tick(World* world, void (*rup_inst_run_callback)(RupInst*))
{
    world_set_block_missing_callback(world, redstone_block_missing);

    // Process all tick boundaries
    Runmap* runmap = runmap_allocate();
    Rup* rup = rup_allocate();
    Rup* new_rup = rup_allocate();
    BlockNode* node;
    FOR_BLOCK_LIST(node, world->blocks, BOUNDARY)
    {
        switch (node->block.material)
        {
            case TORCH:      redstone_torch_update(rup, world, node);      break;
            case REPEATER:   redstone_repeater_update(rup, world, node);   break;
            case COMPARATOR: redstone_comparator_update(rup, world, node); break;
            case PISTON:     redstone_piston_update(rup, world, node);     break;
            default: ERROR("Encountered non power source in the start of the block list");
        }
    }

    // Process all powerable blocks
    while (rup->size > 0)
    {
        for (RupInst* inst = rup->instructions; inst != NULL; inst = inst->next)
        {
            if (inst->command != RUP_POWER)
                continue;

            switch (inst->node->block.material)
            {
                case WIRE:      redstone_wire_update(new_rup, world, inst->node, inst->value.power); break;
                case CONDUCTOR: redstone_conductor_update(new_rup, world, inst->node, inst->value.power); break;
                default:        continue;
            }
        }

        runmap_import(runmap, rup);
        Rup* temp = rup;
        rup = new_rup;
        new_rup = temp;
    }

    // Run update commands
    for (int i = 0; i < RUP_CMD_COUNT; i++)
    for (RupInst* inst = runmap->instructions[i]; inst != NULL; inst = inst->next)
    {
        rup_inst_run_callback(inst);
        rup_inst_run(world, inst);
    }

    // Check for unpowered blocks and reset flags
    redstone_find_unpowered(world, BOUNDARY, rup_inst_run_callback);
    redstone_find_unpowered(world, POWERABLE, rup_inst_run_callback);

    world->ticks++;
    world_clear_block_missing_callback(world);
    rup_free(new_rup);
    rup_free(rup);
    runmap_free(runmap);
}

