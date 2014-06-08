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
#define UPDATE_POWER(NODE,POWER,DELAY) rup_cmd_power(out, world->ticks + (DELAY), node, NODE, POWER)
#define POWER(node) (node)->block.power
#define REPEATER_POWERED(node) (node->block.power_state > node->block.state)
#define MATERIAL_IS(node,name) ((node)->block.material == name)
#define MATERIAL_ISNT(node,name) ((node)->block.material != name)
#define NODE_ADJACENT(node,dir) world_get_adjacent_block(world, node, dir)

static void redstone_conductor_update(World* world, BlockNode* node, RupInst* in, Rup* out)
{
    assert(MATERIAL_IS(node, CONDUCTOR));

    unsigned int power = rup_inst_max_power(in);
    UPDATE_POWER(node, power, 0);

    if (power < MAX_POWER)
        return;

    for (int i = 0; i < DIRECTIONS_COUNT; i++)
    {
        Direction dir = (Direction)i;
        BlockNode* found_node = NODE_ADJACENT(node, dir);
        if (rup_inst_contains_location(in, found_node->block.location))
            continue;

        UPDATE_POWER(found_node, power, 0);
    }
}

static void redstone_wire_update(World* world, BlockNode* node, RupInst* in, Rup* out)
{
    assert(MATERIAL_IS(node, WIRE));

    BlockNode* above = NODE_ADJACENT(node, UP);
    bool covered = above != NULL && MATERIAL_ISNT(above, AIR) && MATERIAL_ISNT(above, EMPTY);

    int new_power = rup_inst_max_power(in);
    UPDATE_POWER(node, new_power, 0);

    if (new_power == 0)
        return;

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

        // Don't pass signal to any block that passed messages to this one
        if (rup_inst_contains_location(in, found_node->block.location))
            continue;

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

        UPDATE_POWER(found_node, wire_power, 0);
    }

    // Block below
    BlockNode* down_node = NODE_ADJACENT(node, DOWN);
    if (MATERIAL_IS(down_node, CONDUCTOR))
        UPDATE_POWER(down_node, new_power, 0);
}

static void redstone_piston_update(World* world, BlockNode* node, RupInst* in, Rup* out)
{
    assert(MATERIAL_IS(node, PISTON));

    unsigned int new_power = rup_inst_max_power(in);
    UPDATE_POWER(node, new_power, 0);

    BlockNode* first = NODE_ADJACENT(node, node->block.direction);
    BlockNode* second = NODE_ADJACENT(first, node->block.direction);

    if (MATERIAL_IS(new_power == 0 ? first : second, AIR))
        rup_cmd_swap(out, 1, node, first, node->block.direction);
}

static void redstone_comparator_update(World* world, BlockNode* node, RupInst* in, Rup* out)
{
    assert(MATERIAL_IS(node, COMPARATOR));

    unsigned int side_power = 0;
    int new_power = 0;
    Location right  = location_move(node->block.location, direction_right(node->block.direction), 1);
    Location left   = location_move(node->block.location, direction_left(node->block.direction), 1);
    Location behind = location_move(node->block.location, direction_invert(node->block.direction), 1);
    
    FOR_RUP_INST(inst, in)
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

static void redstone_repeater_update(World* world, BlockNode* node, RupInst* in, Rup* out)
{
    assert(MATERIAL_IS(node, REPEATER));

    bool side_powered = false;
    int new_power = 0;
    Location right  = location_move(node->block.location, direction_right(node->block.direction), 1);
    Location left   = location_move(node->block.location, direction_left(node->block.direction), 1);
    Location behind = location_move(node->block.location, direction_invert(node->block.direction), 1);
    
    FOR_RUP_INST(inst, in)
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

    if (new_power == 0 || side_powered)
        return;

    // Pass charge to the wire or conductor in front after a delay
    BlockNode* found_node = NODE_ADJACENT(node, node->block.direction);
    UPDATE_POWER(found_node, MAX_POWER, node->block.state + 1);
}

static void redstone_torch_update(World* world, BlockNode* node, RupInst* in, Rup* out)
{
    assert(MATERIAL_IS(node, TORCH));

    unsigned int new_power = 0;
    Location loc_behind = location_move(node->block.location, direction_invert(node->block.direction), 1);
    FOR_RUP_INST(inst, in)
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
        UPDATE_POWER(found_node, MAX_POWER, 1);
    }

    // Pass charge up to a conductor
    BlockNode* up_node = NODE_ADJACENT(node, UP);
    if (MATERIAL_IS(up_node, CONDUCTOR))
        UPDATE_POWER(up_node, MAX_POWER, 1);
}

static void redstone_switch_update(World* world, BlockNode* node, RupInst* in, Rup* out)
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

static RupInst* find_input(World* world, BlockNode* node, Rup* output)
{
    RupInst* insts;
    unsigned int size = 0;

    Bucket* bucket = hashmap_get(world->instructions, node->block.location, false);
    if (bucket == NULL)
    {
        insts = rup_inst_empty_allocate();
        goto end;
    }

    bucket->value = rup_queue_discard_old(bucket->value, world->ticks);
    RupQueue* queue = (RupQueue*)bucket->value;
    if (queue == NULL)
    {
        insts = rup_inst_empty_allocate();
        goto end;
    }

    RupInst* found_insts = rup_queue_find_instructions(queue, world->ticks);
    if (found_insts == NULL)
    {
        insts = rup_inst_empty_allocate();
        goto end;
    }

    size = rup_inst_size(found_insts);
    insts = rup_inst_clone(found_insts, size);

end:
    // Include any instructions generated this tick
    FOR_RUP(rup_node, output)
    {
        if (rup_node->target == node)
            insts = rup_inst_append(insts, size, &rup_node->inst);
    }

    return insts;
}

static void process_output(World* world, BlockNode* node, Rup* input, Rup* output)
{
    FOR_RUP(rup_node, input)
    {
        if (rup_node->tick == world->ticks &&
            !location_equals(rup_node->target->block.location, rup_node->inst.source->block.location))
        {
            rup_remove_by_source(output, rup_node->target);
            block_list_move_after(world->blocks, node, rup_node->target);
        }
    }

    rup_merge(output, input);
}

static void run_output(World* world, Rup* output, void (*inst_run_callback)(RupNode*))
{
    FOR_RUP(rup_node, output)
    {
        if (rup_node->tick == world->ticks &&
            location_equals(rup_node->target->block.location, rup_node->inst.source->block.location))
        {
            world_run_rup(world, rup_node);
            inst_run_callback(rup_node);
            continue;
        }

        Bucket* bucket = hashmap_get(world->instructions, rup_node->target->block.location, true);
        RupQueue* queue = (RupQueue*)bucket->value;
        if (queue == NULL)
        {
            queue = rup_queue_allocate(rup_node->tick);
            bucket->value = queue;
        }
        else
        {
            queue = rup_queue_find(queue, rup_node->tick);
            if (queue == NULL)
            {
                queue = rup_queue_allocate(rup_node->tick);
                queue->next = bucket->value;
                bucket->value = queue;
            }
        }

        if (rup_queue_find_inst(queue, &rup_node->inst) == NULL)
           rup_queue_add(queue, &rup_node->inst);
    }
}

void redstone_tick(World* world, void (*inst_run_callback)(RupNode*), unsigned int count)
{
    world_set_block_missing_callback(world, redstone_block_missing);

    for (unsigned int i = 0; i < count; i++)
    {
        unsigned int loops = 0;
        Rup output = rup_empty();

        FOR_BLOCK_LIST(world->blocks)
        {
            RupInst* in = find_input(world, node, &output);
            if (in == NULL)
            {
                in = malloc(sizeof(Rup));
                *in = rup_inst_create(RUP_HALT, NULL);
            }
            Rup out = rup_empty();

            switch (node->block.material)
            {
                case EMPTY:      free(in); continue;
                case AIR:        free(in); continue;
                case INSULATOR:  free(in); continue;
                case WIRE:       redstone_wire_update      (world, node, in, &out); break;
                case CONDUCTOR:  redstone_conductor_update (world, node, in, &out); break;
                case TORCH:      redstone_torch_update     (world, node, in, &out); break;
                case PISTON:     redstone_piston_update    (world, node, in, &out); break;
                case REPEATER:   redstone_repeater_update  (world, node, in, &out); break;
                case COMPARATOR: redstone_comparator_update(world, node, in, &out); break;
                case SWITCH:     redstone_switch_update    (world, node, in, &out); break;
                default: ERROR("Encountered unknown block material");
            }
            free(in);

            process_output(world, node, &out, &output);

            loops++;
            if (loops > world->blocks->size * 2)
            {
                printf("Error: Logic loop detected while performing tick.\n");
                break;
            }
        }

        run_output(world, &output, inst_run_callback);
        rup_free(&output);
        world->ticks++;
    }

    world_clear_block_missing_callback(world);
}

