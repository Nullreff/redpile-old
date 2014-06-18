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
#include "redstone.h"

#define MAX_POWER 15

#define LOCATION(NODE)  ((NODE)->location)
#define MATERIAL(NODE)  ((NODE)->type)
#define POWER(NODE)     FIELD_GET(NODE, 0)
#define DIRECTION(NODE) FIELD_GET(NODE, 1)
#define STATE(NODE)     FIELD_GET(NODE, 2)

#define NODE_ADJACENT(NODE,DIR) world_get_adjacent_node(world, NODE, DIR)
#define MOVE_TO_NODE(NODE,DIR) NODE = NODE_ADJACENT(NODE, DIR)

#define SEND_POWER(NODE,POWER,DELAY) rup_cmd_power(out, world->ticks + (DELAY), node, NODE, POWER)
#define SEND_MOVE(NODE,DIR,DELAY) rup_cmd_move(out, world->ticks + (DELAY), node, NODE, DIR)
#define CMD_POWER(POWER) SEND_POWER(node, POWER, 0)
#define CMD_MOVE(DIR) SEND_MOVE(node, DIR, 0)
#define CMD_REMOVE() rup_cmd_remove(out, world->ticks, node, node)

static void redstone_conductor_update(World* world, Node* node, RupInst* in, Rup* out)
{
    assert(MATERIAL(node) == CONDUCTOR);

    RupInst* move_inst = rup_inst_find_move(in);
    if (move_inst != NULL)
    {
        CMD_MOVE(move_inst->value.direction);
        return;
    }

    unsigned int power = rup_inst_max_power(in);
    SEND_POWER(node, power, 0);

    if (power < MAX_POWER)
        return;

    for (int i = 0; i < DIRECTIONS_COUNT; i++)
    {
        Direction dir = (Direction)i;
        Node* found_node = NODE_ADJACENT(node, dir);
        if (rup_inst_power_check(in, LOCATION(found_node), power))
            SEND_POWER(found_node, power, 0);
    }
}

static void redstone_wire_update(World* world, Node* node, RupInst* in, Rup* out)
{
    assert(MATERIAL(node) == WIRE);

    RupInst* move_inst = rup_inst_find_move(in);
    if (move_inst != NULL)
    {
        CMD_REMOVE();
        return;
    }

    Node* above = NODE_ADJACENT(node, UP);
    bool covered = above != NULL && MATERIAL(above) != AIR && MATERIAL(above) != EMPTY;

    int new_power = rup_inst_max_power(in);
    SEND_POWER(node, new_power, 0);

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
        Node* found_node = NODE_ADJACENT(node, dir);

        // Down one block
        if (MATERIAL(found_node) == AIR)
        {
            MOVE_TO_NODE(found_node, DOWN);
            if (MATERIAL(found_node) != WIRE)
                continue;
        }
        // Up one block
        else if (MATERIAL(found_node) == CONDUCTOR)
        {
            // Add charge to the block
            Node* right = NODE_ADJACENT(node, direction_left(dir));
            Node* left = NODE_ADJACENT(node, direction_right(dir));
            Location behind = location_move(LOCATION(node), direction_invert(dir), 1);

            if (MATERIAL(right) != WIRE && MATERIAL(left) != WIRE &&
                rup_inst_power_check(in, LOCATION(found_node), wire_power))
            {
                SEND_POWER(found_node, wire_power, 0);
            }

            if (covered)
                continue;

            MOVE_TO_NODE(found_node, UP);
            if (MATERIAL(found_node) != WIRE)
                continue;
        }

        if (rup_inst_power_check(in, LOCATION(found_node), wire_power))
            SEND_POWER(found_node, wire_power, 0);
    }

    // Block below
    Node* down_node = NODE_ADJACENT(node, DOWN);

    if (MATERIAL(down_node) == CONDUCTOR &&
        rup_inst_power_check(in, LOCATION(down_node), new_power))
    {
        SEND_POWER(down_node, new_power, 0);
    }
}

#define RETRACTED 0
#define RETRACTING 1
#define EXTENDED 2
#define EXTENDING 3
static void redstone_piston_update(World* world, Node* node, RupInst* in, Rup* out)
{
    assert(MATERIAL(node) == PISTON);

    Node* first = NODE_ADJACENT(node, DIRECTION(node));
    Node* second = NODE_ADJACENT(first, DIRECTION(node));

    unsigned int new_power = rup_inst_max_power(in);

    unsigned int state;
    if (new_power == 0)
    {
        if (MATERIAL(first) == AIR)
            state = RETRACTING;
        else
            state = RETRACTED;
    }
    else
    {
        if (MATERIAL(second) == AIR)
            state = EXTENDING;
        else
            state = EXTENDED;
    }

    RupInst* move_inst = rup_inst_find_move(in);
    if (move_inst != NULL && state == RETRACTED)
    {
        CMD_MOVE(move_inst->value.direction);
        return;
    }

    SEND_POWER(node, new_power, 0);

    if (state == EXTENDING)
        SEND_MOVE(first, DIRECTION(node), 1);
    else if (state == RETRACTING)
        SEND_MOVE(second, direction_invert(DIRECTION(node)), 1);
}

static void redstone_comparator_update(World* world, Node* node, RupInst* in, Rup* out)
{
    assert(MATERIAL(node) == COMPARATOR);

    RupInst* move_inst = rup_inst_find_move(in);
    if (move_inst != NULL)
    {
        CMD_REMOVE();
        return;
    }

    unsigned int side_power = 0;
    int new_power = 0;
    Location right  = location_move(LOCATION(node), direction_right(DIRECTION(node)), 1);
    Location left   = location_move(LOCATION(node), direction_left(DIRECTION(node)), 1);
    Location behind = location_move(LOCATION(node), direction_invert(DIRECTION(node)), 1);
    
    FOR_RUP_INST(inst, in)
    {
        // Power coming from the side
        if ((location_equals(LOCATION(inst->source), right) ||
             location_equals(LOCATION(inst->source), left)) &&
            side_power < inst->value.power)
        {
            side_power = inst->value.power;
        }

        // Power coming from behind
        if (location_equals(LOCATION(inst->source), behind))
            new_power = inst->value.power;
    }

    SEND_POWER(node, new_power, 0);

    if (new_power == 0)
        return;

    int change = new_power;

    // Subtraction mode
    if (STATE(node) > 0)
        change -= side_power;

    new_power = new_power > side_power ? change : 0;

    if (new_power == 0)
        return;

    // Pass charge to the wire or conductor in front
    Node* found_node = NODE_ADJACENT(node, DIRECTION(node));
    SEND_POWER(found_node, new_power, 1);
}

static void redstone_repeater_update(World* world, Node* node, RupInst* in, Rup* out)
{
    assert(MATERIAL(node) == REPEATER);

    RupInst* move_inst = rup_inst_find_move(in);
    if (move_inst != NULL)
    {
        CMD_REMOVE();
        return;
    }

    bool side_powered = false;
    int new_power = 0;
    Location right  = location_move(LOCATION(node), direction_right(DIRECTION(node)), 1);
    Location left   = location_move(LOCATION(node), direction_left(DIRECTION(node)), 1);
    Location behind = location_move(LOCATION(node), direction_invert(DIRECTION(node)), 1);
    
    FOR_RUP_INST(inst, in)
    {
        // Power coming from the side
        if (MATERIAL(inst->source) == REPEATER &&
            (location_equals(LOCATION(inst->source), right) ||
            location_equals(LOCATION(inst->source), left)))
        {
            side_powered = (inst->value.power > 0) || side_powered;
        }

        // Power coming from behind
        if (location_equals(LOCATION(inst->source), behind))
            new_power = inst->value.power;
    }

    SEND_POWER(node, new_power, 0);

    if (new_power == 0 || side_powered)
        return;

    // Pass charge to the wire or conductor in front after a delay
    Node* found_node = NODE_ADJACENT(node, DIRECTION(node));
    SEND_POWER(found_node, MAX_POWER, STATE(node) + 1);
}

static void redstone_torch_update(World* world, Node* node, RupInst* in, Rup* out)
{
    assert(MATERIAL(node) == TORCH);

    RupInst* move_inst = rup_inst_find_move(in);
    if (move_inst != NULL)
    {
        CMD_REMOVE();
        return;
    }

    unsigned int new_power = 0;
    Location loc_behind = location_move(LOCATION(node), direction_invert(DIRECTION(node)), 1);
    FOR_RUP_INST(inst, in)
    {
        // Power coming from behind
        if (location_equals(LOCATION(inst->source), loc_behind))
            new_power = inst->value.power;
    }

    SEND_POWER(node, new_power, 0);

    if (new_power > 0)
        return;

    // Pass charge to any adjacent wires
    Direction behind = direction_invert(DIRECTION(node));
    Direction directions[5] = {NORTH, SOUTH, EAST, WEST, DOWN};
    for (int i = 0; i < 5; i++)
    {
        Direction dir = directions[i];
        if (dir == behind)
            continue;

        Node* found_node = NODE_ADJACENT(node, dir);
        SEND_POWER(found_node, MAX_POWER, 1);
    }

    // Pass charge up to a conductor
    Node* up_node = NODE_ADJACENT(node, UP);
    if (MATERIAL(up_node) == CONDUCTOR)
        SEND_POWER(up_node, MAX_POWER, 1);
}

static void redstone_switch_update(World* world, Node* node, RupInst* in, Rup* out)
{
    assert(MATERIAL(node) == SWITCH);

    RupInst* move_inst = rup_inst_find_move(in);
    if (move_inst != NULL)
    {
        CMD_REMOVE();
        return;
    }

    if (STATE(node) == 0)
        return;

    Direction behind = direction_invert(DIRECTION(node));
    for (Direction dir = (Direction)0; dir < DIRECTIONS_COUNT; dir++)
    {
        Node* found_node = NODE_ADJACENT(node, dir);
        if (MATERIAL(found_node) != CONDUCTOR || dir == behind)
            SEND_POWER(found_node, MAX_POWER, 1);
    }

}

static bool redstone_node_missing(Location location, Type* type)
{
    *type = AIR;
    return true;
}

static RupInst* find_input(World* world, Node* node, Rup* output)
{
    RupInst* insts = world_find_instructions(world, node);
    unsigned int size = 0;

    if (insts != NULL)
    {
        size = rup_inst_size(insts);
        insts = rup_inst_clone(insts, size);
    }
    else
    {
        insts = rup_inst_empty_allocate();
    }

    // Include any instructions generated this tick
    FOR_RUP(rup_node, output)
    {
        if (rup_node->target == node)
            insts = rup_inst_append(insts, size++, &rup_node->inst);
    }

    return insts;
}

static void process_output(World* world, Node* node, Rup* input, Rup* output)
{
    FOR_RUP(rup_node, input)
    {
        if (rup_node->tick == world->ticks &&
            !location_equals(LOCATION(rup_node->target), LOCATION(rup_node->inst.source)) &&
            !rup_contains(output, rup_node))
        {
            rup_remove_by_source(output, rup_node->target);
            node_list_move_after(world->nodes, node, rup_node->target);
        }
    }

    rup_merge(output, input);
}

static void run_output(World* world, Rup* output, void (*inst_run_callback)(RupNode*))
{
    FOR_RUP(rup_node, output)
    {
        if (rup_node->tick == world->ticks &&
            location_equals(LOCATION(rup_node->target), LOCATION(rup_node->inst.source)))
        {
            inst_run_callback(rup_node);
            world_run_rup(world, rup_node);
            continue;
        }

        Bucket* bucket = hashmap_get(world->instructions, LOCATION(rup_node->target), true);
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

        rup_queue_add(queue, &rup_node->inst);
    }
}

void redstone_tick(World* world, void (*inst_run_callback)(RupNode*), unsigned int count)
{
    world_set_node_missing_callback(world, redstone_node_missing);

    for (unsigned int i = 0; i < count; i++)
    {
        unsigned int loops = 0;
        Rup output = rup_empty();

        FOR_NODE_LIST(world->nodes)
        {
            RupInst* in = find_input(world, node, &output);
            Rup out = rup_empty();

            switch (MATERIAL(node))
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
            if (loops > world->nodes->size * 2)
            {
                printf("Error: Logic loop detected while performing tick.\n");
                break;
            }
        }

        run_output(world, &output, inst_run_callback);
        rup_free(&output);
        world->ticks++;
    }

    world_clear_node_missing_callback(world);
}

