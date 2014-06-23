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

#define LOWER_POWER(NODE,POWER) rup_insts_power_check(in, LOCATION(NODE), POWER)
#define NODE_ADJACENT(NODE,DIR) world_get_adjacent_node(world, NODE, DIR)
#define MOVE_TO_NODE(NODE,DIR) NODE = NODE_ADJACENT(NODE, DIR)

#define SEND_POWER(NODE,POWER,DELAY) rup_cmd_power (messages,  world->ticks + (DELAY), node, NODE, POWER)
#define SEND_MOVE(NODE,DIR,DELAY)    rup_cmd_move  (messages,  world->ticks + (DELAY), node, NODE, DIR  )
#define CMD_POWER(POWER)             rup_cmd_power (sets,      world->ticks,           node, node, POWER)
#define CMD_MOVE(DIR)                rup_cmd_move  (sets,      world->ticks,           node, node, DIR  )
#define CMD_REMOVE()                 rup_cmd_remove(sets,      world->ticks,           node, node       )

#define RUP_METHOD(TYPE)\
    static void redstone_ ## TYPE ## _update(World* world, Node* node, RupInsts* in, Rup* messages, Rup* sets)

#define RUP_REGISTER(TYPE)\
    case TYPE: redstone_ ## TYPE ## _update(world, node, in, &output, &sets); break

RUP_METHOD(EMPTY) {}
RUP_METHOD(AIR) {}
RUP_METHOD(INSULATOR) {}

RUP_METHOD(CONDUCTOR)
{
    RupInst* move_inst = rup_insts_find_move(in);
    if (move_inst != NULL)
    {
        CMD_MOVE(move_inst->value.direction);
        return;
    }

    unsigned int power = rup_insts_max_power(in);
    CMD_POWER(power);
    if (power == 0)
        return;

    bool max_powered = power == MAX_POWER;

    for (int i = 0; i < DIRECTIONS_COUNT; i++)
    {
        Direction dir = (Direction)i;
        Node* found_node = NODE_ADJACENT(node, dir);
        if (MATERIAL(found_node) == CONDUCTOR)
            continue;

        if (!max_powered && MATERIAL(found_node) == WIRE)
            continue;

        if (LOWER_POWER(found_node, power))
            SEND_POWER(found_node, power, 0);
    }
}

RUP_METHOD(WIRE)
{
    RupInst* move_inst = rup_insts_find_move(in);
    if (move_inst != NULL)
    {
        CMD_REMOVE();
        return;
    }

    Node* above = NODE_ADJACENT(node, UP);
    bool covered = above != NULL && MATERIAL(above) != AIR && MATERIAL(above) != EMPTY;

    int new_power = rup_insts_max_power(in);
    CMD_POWER(new_power);

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

            if (MATERIAL(right) != WIRE && MATERIAL(left) != WIRE &&
                LOWER_POWER(found_node, wire_power))
            {
                SEND_POWER(found_node, wire_power, 0);
            }

            if (covered)
                continue;

            MOVE_TO_NODE(found_node, UP);
            if (MATERIAL(found_node) != WIRE)
                continue;
        }

        if (LOWER_POWER(found_node, wire_power))
            SEND_POWER(found_node, wire_power, 0);
    }

    // Block below
    Node* down_node = NODE_ADJACENT(node, DOWN);
    if (MATERIAL(down_node) == CONDUCTOR && LOWER_POWER(down_node, new_power))
        SEND_POWER(down_node, new_power, 0);
}

#define RETRACTED 0
#define RETRACTING 1
#define EXTENDED 2
#define EXTENDING 3
RUP_METHOD(PISTON)
{
    Node* first = NODE_ADJACENT(node, DIRECTION(node));
    Node* second = NODE_ADJACENT(first, DIRECTION(node));

    unsigned int new_power = rup_insts_max_power(in);

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

    RupInst* move_inst = rup_insts_find_move(in);
    if (move_inst != NULL && state == RETRACTED)
    {
        CMD_MOVE(move_inst->value.direction);
        return;
    }

    CMD_POWER(new_power);

    if (state == EXTENDING)
        SEND_MOVE(first, DIRECTION(node), 1);
    else if (state == RETRACTING)
        SEND_MOVE(second, direction_invert(DIRECTION(node)), 1);
}

RUP_METHOD(COMPARATOR)
{
    RupInst* move_inst = rup_insts_find_move(in);
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
    
    for (int i = 0; i < in->size; i++)
    {
        RupInst* inst = in->data + i;

        // Power coming from the side
        if ((LOCATION_EQUALS(inst->source, right) ||
             LOCATION_EQUALS(inst->source, left)) &&
            side_power < inst->value.power)
        {
            side_power = inst->value.power;
        }

        // Power coming from behind
        if (LOCATION_EQUALS(inst->source, behind))
            new_power = inst->value.power;
    }

    CMD_POWER(new_power);

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

RUP_METHOD(REPEATER)
{
    RupInst* move_inst = rup_insts_find_move(in);
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
    
    for (int i = 0; i < in->size; i++)
    {
        RupInst* inst = in->data + i;

        // Power coming from the side
        if (inst->source_material == REPEATER &&
            (LOCATION_EQUALS(inst->source, right) ||
            LOCATION_EQUALS(inst->source, left)))
        {
            side_powered = (inst->value.power > 0) || side_powered;
        }

        // Power coming from behind
        if (LOCATION_EQUALS(inst->source, behind))
            new_power = inst->value.power;
    }

    CMD_POWER(new_power);

    if (new_power == 0 || side_powered)
        return;

    // Pass charge to the wire or conductor in front after a delay
    Node* found_node = NODE_ADJACENT(node, DIRECTION(node));
    SEND_POWER(found_node, MAX_POWER, STATE(node) + 1);
}

RUP_METHOD(TORCH)
{
    RupInst* move_inst = rup_insts_find_move(in);
    if (move_inst != NULL)
    {
        CMD_REMOVE();
        return;
    }

    unsigned int new_power = 0;
    Location loc_behind = location_move(LOCATION(node), direction_invert(DIRECTION(node)), 1);
    for (int i = 0; i < in->size; i++)
    {
        RupInst* inst = in->data + i;

        // Power coming from behind
        if (LOCATION_EQUALS(inst->source, loc_behind))
            new_power = inst->value.power;
    }

    CMD_POWER(new_power);

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

RUP_METHOD(SWITCH)
{
    RupInst* move_inst = rup_insts_find_move(in);
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

static RupInsts* find_input(World* world, Node* node, Rup* messages)
{
    RupInsts* found_insts = world_find_messages(world, node);
    RupInsts* insts = found_insts != NULL ? rup_insts_clone(found_insts) : rup_insts_allocate();

    // Include any instructions generated this tick
    return rup_insts_append_nodes(insts, messages, node->location);
}

static void process_output(World* world, Node* node, Rup* output, Rup* messages_out, Rup* sets_out)
{
    FOR_RUP(rup_node, output)
    {
        if (rup_node->tick == world->ticks && !rup_contains(messages_out, rup_node))
        {
            assert(!LOCATION_EQUALS(LOCATION(rup_node->target), rup_node->inst.source));
            rup_remove_by_source(messages_out, rup_node->target->location);
            rup_remove_by_source(sets_out, rup_node->target->location);
            node_list_move_after(world->nodes, node, rup_node->target);
        }
    }

    rup_merge(messages_out, output);
}

static void run_messages(World* world, Rup* messages, void (*inst_run_callback)(RupNode*))
{
    FOR_RUP(message, messages)
    {
        assert(!LOCATION_EQUALS(LOCATION(message->target), message->inst.source));

        Bucket* bucket = hashmap_get(world->messages, LOCATION(message->target), true);
        RupQueue* queue = (RupQueue*)bucket->value;
        if (queue == NULL)
        {
            queue = rup_queue_allocate(message->tick);
            bucket->value = queue;
        }
        else
        {
            queue = rup_queue_find(queue, message->tick);
            if (queue == NULL)
            {
                queue = rup_queue_allocate(message->tick);
                queue->next = bucket->value;
                bucket->value = queue;
            }
        }

        rup_queue_add(queue, &message->inst);
    }
}

static void run_sets(World* world, Rup* sets, void (*inst_run_callback)(RupNode*))
{
    FOR_RUP(set, sets)
    {
        inst_run_callback(set);
        world_run_rup(world, set);
    }
}

void redstone_tick(World* world, void (*inst_run_callback)(RupNode*), unsigned int count, bool verbose)
{
    world_set_node_missing_callback(world, redstone_node_missing);

    for (unsigned int i = 0; i < count; i++)
    {
        if (verbose)
            printf("--- Tick %llu ---\n", world->ticks);

        unsigned int loops = 0;
        Rup messages = rup_empty(true, true, world->hashmap->size);
        Rup sets = rup_empty(false, true, world->hashmap->size);

        FOR_NODE_LIST(world->nodes)
        {
            RupInsts* in = find_input(world, node, &messages);
            Rup output = rup_empty(false, false, 0);

            switch (MATERIAL(node))
            {
                RUP_REGISTER(EMPTY);
                RUP_REGISTER(AIR);
                RUP_REGISTER(INSULATOR);
                RUP_REGISTER(WIRE);
                RUP_REGISTER(CONDUCTOR);
                RUP_REGISTER(TORCH);
                RUP_REGISTER(PISTON);
                RUP_REGISTER(REPEATER);
                RUP_REGISTER(COMPARATOR);
                RUP_REGISTER(SWITCH);
                default: ERROR("Encountered unknown block material");
            }
            free(in);

            process_output(world, node, &output, &messages, &sets);

            loops++;
            if (loops > world->nodes->size * 2)
            {
                printf("Error: Logic loop detected while performing tick.\n");
                break;
            }
        }

        if (verbose)
        {
            printf("Messages:\n");
            FOR_RUP(message, &messages)
            {
                if (message->tick == world->ticks)
                    rup_node_print_verbose(message, world->ticks);
            }

            printf("Queued:\n");
            FOR_RUP(message, &messages)
            {
                if (message->tick > world->ticks)
                    rup_node_print_verbose(message, world->ticks);
            }
            printf("Output:\n");
        }

        run_messages(world, &messages, inst_run_callback);
        run_sets(world, &sets, inst_run_callback);
        
        rup_free(&messages);
        rup_free(&sets);
        world->ticks++;
    }

    world_clear_node_missing_callback(world);
}

