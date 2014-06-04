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
#define UPDATE_POWER(NODE,POWER,DELAY) rup_cmd_power(out, DELAY, node, NODE, POWER)
#define POWER(node) (node)->block.power
#define REPEATER_POWERED(node) (node->block.power_state > node->block.state)
#define MATERIAL_IS(node,name) ((node)->block.material == name)
#define MATERIAL_ISNT(node,name) ((node)->block.material != name)
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

static void redstone_conductor_update(World* world, BlockNode* node, Rup* in, Rup* out)
{
    assert(MATERIAL_IS(node, CONDUCTOR));

    unsigned int power = rup_max_power(in);
    UPDATE_POWER(node, power, 0);

    if (power < MAX_POWER)
        return;

    for (int i = 0; i < DIRECTIONS_COUNT; i++)
    {
        Direction dir = (Direction)i;
        BlockNode* found_node = NODE_ADJACENT(node, dir);
        if (can_power_from_behind(found_node, dir))
            UPDATE_POWER(found_node, power, 0);
    }
}

static void redstone_wire_update(World* world, BlockNode* node, Rup* in, Rup* out)
{
    assert(MATERIAL_IS(node, WIRE));

    BlockNode* above = NODE_ADJACENT(node, UP);
    bool covered = above != NULL && MATERIAL_ISNT(above, AIR) && MATERIAL_ISNT(above, EMPTY);

    int new_power = rup_max_power(in);
    UPDATE_POWER(node, new_power, 0);

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
                UPDATE_POWER(found_node, new_power, 0);

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

        UPDATE_POWER(found_node, wire_power, 0);
    }

    // Block below
    BlockNode* down_node = NODE_ADJACENT(node, DOWN);
    if (MATERIAL_IS(down_node, CONDUCTOR))
        UPDATE_POWER(down_node, new_power, 0);
}

static void redstone_piston_update(World* world, BlockNode* node, Rup* in, Rup* out)
{
    assert(MATERIAL_IS(node, PISTON));

    unsigned int new_power = rup_max_power(in);
    UPDATE_POWER(node, new_power, 0);

    BlockNode* first = NODE_ADJACENT(node, node->block.direction);
    BlockNode* second = NODE_ADJACENT(first, node->block.direction);

    if (MATERIAL_IS(new_power == 0 ? first : second, AIR))
        rup_cmd_swap(out, 1, node, first, second);
}

static void redstone_comparator_update(World* world, BlockNode* node, Rup* in, Rup* out)
{
    assert(MATERIAL_IS(node, COMPARATOR));

    unsigned int side_power = 0;
    int new_power = 0;
    Location right  = location_move(node->block.location, direction_right(node->block.direction), 1);
    Location left   = location_move(node->block.location, direction_left(node->block.direction), 1);
    Location behind = location_move(node->block.location, direction_invert(node->block.direction), 1);
    
    FOR_RUP(in)
    {
        // Power coming from the side
        if ((location_equals(inst->source->block.location, right) ||
             location_equals(inst->source->block.location, left)) &&
            side_power < inst->value.power)
        {
            side_power = inst->value.power;
        }

        // Power coming from behind
        if (location_equals(inst->source->block.location, behind))
            new_power = inst->value.power;
    }

    UPDATE_POWER(node, new_power, 0);

    if (new_power == 0)
        return;

    int change = new_power;

    // Subtraction mode
    if (node->block.state > 0)
        change -= side_power;

    new_power = new_power > side_power ? change : 0;

    if (new_power == 0)
        return;

    // Pass charge to the wire or conductor in front
    BlockNode* found_node = NODE_ADJACENT(node, node->block.direction);
    UPDATE_POWER(found_node, new_power, 1);
}

static void redstone_repeater_update(World* world, BlockNode* node, Rup* in, Rup* out)
{
    assert(MATERIAL_IS(node, REPEATER));

    bool side_powered = false;
    int new_power = 0;
    Location right  = location_move(node->block.location, direction_right(node->block.direction), 1);
    Location left   = location_move(node->block.location, direction_left(node->block.direction), 1);
    Location behind = location_move(node->block.location, direction_invert(node->block.direction), 1);
    
    FOR_RUP(in)
    {
        // Power coming from the side
        if (inst->source->block.material == REPEATER &&
            (location_equals(inst->source->block.location, right) ||
            location_equals(inst->source->block.location, left)))
        {
            side_powered = (inst->value.power > 0) || side_powered;
        }

        // Power coming from behind
        if (location_equals(inst->source->block.location, behind))
            new_power = inst->value.power;
    }

    UPDATE_POWER(node, new_power, 0);

    if (side_powered)
        return;

    // Pass charge to the wire or conductor in front after a delay
    BlockNode* found_node = NODE_ADJACENT(node, node->block.direction);
    UPDATE_POWER(found_node, MAX_POWER, node->block.state + 1);
}

static void redstone_torch_update(World* world, BlockNode* node, Rup* in, Rup* out)
{
    assert(MATERIAL_IS(node, TORCH));

    unsigned int new_power = 0;
    Location loc_behind = location_move(node->block.location, direction_invert(node->block.direction), 1);
    FOR_RUP(in)
    {
        // Power coming from behind
        if (location_equals(inst->source->block.location, loc_behind))
            new_power = inst->value.power;
    }

    UPDATE_POWER(node, new_power, 0);

    if (new_power > 0)
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
            UPDATE_POWER(found_node, MAX_POWER, 1);
    }

    // Pass charge up to a conductor
    BlockNode* up_node = NODE_ADJACENT(node, UP);
    if (MATERIAL_IS(up_node, CONDUCTOR))
        UPDATE_POWER(up_node, MAX_POWER, 1);
}

static void redstone_switch_update(World* world, BlockNode* node, Rup* in, Rup* out)
{
    assert(MATERIAL_IS(node, SWITCH));

    if (node->block.state == 0)
        return;

    Direction behind = direction_invert(node->block.direction);
    for (Direction dir = (Direction)0; dir < DIRECTIONS_COUNT; dir++)
    {
        BlockNode* found_node = NODE_ADJACENT(node, dir);
        if (MATERIAL_ISNT(found_node, CONDUCTOR) || dir == behind)
            UPDATE_POWER(found_node, MAX_POWER, 1);
    }

}

static bool redstone_block_missing(Block* block)
{
    block->material = AIR;
    return true;
}

void redstone_tick(World* world, void (*inst_run_callback)(RupInst*), unsigned int count)
{
    world_set_block_missing_callback(world, redstone_block_missing);

    for (unsigned int i = 0; i < count; i++)
    {
        FOR_BLOCK_LIST(world->blocks)
        {
            Rup in = rup_empty();
            Rup out = rup_empty();

            Bucket* bucket = hashmap_get(world->instructions, node->block.location, false);
            if (bucket != NULL)
            {
                // Process the 'in' rup
                // 1. De-increment the delay on all instructions
                // 2. Any less than zero should be discarded
                // 3. Any at zero should be inserted into the 'in' rup
            }

            switch (node->block.material)
            {
                case EMPTY:      break;
                case AIR:        break;
                case INSULATOR:  break;
                case WIRE:       redstone_wire_update      (world, node, &in, &out); break;
                case CONDUCTOR:  redstone_conductor_update (world, node, &in, &out); break;
                case TORCH:      redstone_torch_update     (world, node, &in, &out); break;
                case PISTON:     redstone_piston_update    (world, node, &in, &out); break;
                case REPEATER:   redstone_repeater_update  (world, node, &in, &out); break;
                case COMPARATOR: redstone_comparator_update(world, node, &in, &out); break;
                case SWITCH:     redstone_switch_update    (world, node, &in, &out); break;
                default: ERROR("Encountered unknown block material");
            }

            // Process the 'out' rup
            // 1. Any instructions that target the current node with a delay of zero should be printed
            // 2. Any instructions that have a delay of zero but target another node should flag their target for re-execution
            // 3. All other instructions are added to the queue for upcoming ticks
        }
    }

    world->ticks += count;
    world_clear_block_missing_callback(world);
}

