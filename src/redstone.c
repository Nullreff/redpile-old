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

#define SEND_POWER(NODE,POWER,DELAY) queue_push_inst(messages, RUP_POWER,  world->ticks + (DELAY), node, NODE, POWER)
#define SEND_MOVE(NODE,DIR,DELAY)    queue_push_inst(messages, RUP_MOVE,   world->ticks + (DELAY), node, NODE, DIR  )
#define CMD_POWER(POWER)             queue_push_inst(sets,     RUP_POWER,  world->ticks,           node, node, POWER)
#define CMD_MOVE(DIR)                queue_push_inst(sets,     RUP_MOVE,   world->ticks,           node, node, DIR  )
#define CMD_REMOVE()                 queue_push_inst(sets,     RUP_REMOVE, world->ticks,           node, node, 0    )

#define RUP_METHOD(TYPE)\
    static void redstone_ ## TYPE ## _update(World* world, Node* node, RupInsts* in, Queue* messages, Queue* sets)

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
        CMD_MOVE(move_inst->message);
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
        CMD_MOVE(move_inst->message);
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
        if ((LOCATION_EQUALS(inst->source.location, right) ||
             LOCATION_EQUALS(inst->source.location, left)) &&
            side_power < inst->message)
        {
            side_power = inst->message;
        }

        // Power coming from behind
        if (LOCATION_EQUALS(inst->source.location, behind))
            new_power = inst->message;
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
        if (inst->source.type == REPEATER &&
            (LOCATION_EQUALS(inst->source.location, right) ||
            LOCATION_EQUALS(inst->source.location, left)))
        {
            side_powered = (inst->message > 0) || side_powered;
        }

        // Power coming from behind
        if (LOCATION_EQUALS(inst->source.location, behind))
            new_power = inst->message;
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
        if (LOCATION_EQUALS(inst->source.location, loc_behind))
            new_power = inst->message;
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
            SEND_POWER(found_node, MAX_POWER, 0);
    }

}

static bool redstone_node_missing(Location location, Type* type)
{
    *type = AIR;
    return true;
}

static RupInsts* find_input(World* world, Node* node, Queue* messages)
{
    QueueNode* found = queue_find_nodes(messages, node, world->ticks);
    int new_messages = 0;
    QueueNode* iter = found;
    while (iter != NULL &&
           iter->data.tick == world->ticks &&
           iter->data.target.node == node)
    {
        new_messages++;
        iter = iter->next;
    }

    RupInsts* found_insts = world_find_messages(world, node);
    int total = (found_insts != NULL ? found_insts->size : 0) + new_messages;

    RupInsts* insts = rup_insts_allocate(total);
    if (total == 0)
        return insts;

    for (int i = 0; i < new_messages; i++)
    {
        insts->data[i] = rup_inst_create(&found->data);
        found = found->next;
    }

    if (found_insts != NULL)
        rup_insts_copy(insts->data + new_messages, found_insts);

    if (world->max_inputs < total)
        world->max_inputs = total;

    return insts;
}

static void process_output(World* world, Node* node, Queue* output, Queue* messages_out, Queue* sets_out)
{
    FOR_QUEUE(queue_node, output)
    {
        if (queue_node->data.tick == world->ticks && !queue_contains(messages_out, queue_node))
        {
            assert(!LOCATION_EQUALS(queue_node->data.target.location, queue_node->data.source.location));
            queue_remove_source(messages_out, queue_node->data.target.location);
            queue_remove_source(sets_out, queue_node->data.target.location);
            node_list_move_after(world->nodes, node, queue_node->data.target.node);
        }
    }

    queue_merge(messages_out, output);
}

static void run_messages(World* world, Queue* messages)
{
    FOR_QUEUE(message, messages)
    {
        assert(!LOCATION_EQUALS(message->data.target.location, message->data.source.location));

        Bucket* bucket = hashmap_get(world->messages, message->data.target.location, true);
        RupQueue* queue = (RupQueue*)bucket->value;
        if (queue == NULL)
        {
            queue = rup_queue_allocate(message->data.tick);
            bucket->value = queue;
        }
        else
        {
            queue = rup_queue_find(queue, message->data.tick);
            if (queue == NULL)
            {
                queue = rup_queue_allocate(message->data.tick);
                queue->next = bucket->value;
                bucket->value = queue;
            }
        }

        rup_queue_add(queue, &message->data);
    }
}

static void run_sets(World* world, Queue* sets, LogLevel log_level)
{
    FOR_QUEUE(set, sets)
    {
        if (log_level != SILENT)
            queue_data_print(&set->data, message_type_print);
        world_run_data(world, &set->data);
    }
}

void redstone_tick(World* world, unsigned int count, LogLevel log_level)
{
    world_set_node_missing_callback(world, redstone_node_missing);

    for (unsigned int i = 0; i < count; i++)
    {
        if (log_level == VERBOSE)
            printf("--- Tick %llu ---\n", world->ticks);

        unsigned int loops = 0;
        Queue messages = queue_empty(true, true, world->hashmap->size);
        Queue sets = queue_empty(false, true, world->hashmap->size);

        FOR_NODE_LIST(world->nodes)
        {
            RupInsts* in = find_input(world, node, &messages);
            Queue output = queue_empty(false, false, 0);

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

        if (log_level == VERBOSE)
        {
            printf("Messages:\n");
            FOR_QUEUE(message, &messages)
            {
                if (message->data.tick == world->ticks)
                    queue_data_print_verbose(&message->data, message_type_print, world->ticks);
            }

            printf("Queued:\n");
            FOR_QUEUE(message, &messages)
            {
                if (message->data.tick > world->ticks)
                    queue_data_print_verbose(&message->data, message_type_print, world->ticks);
            }
            printf("Output:\n");
        }

        run_messages(world, &messages);
        run_sets(world, &sets, log_level);
        
        queue_free(&messages);
        queue_free(&sets);
        world->ticks++;
    }

    world_clear_node_missing_callback(world);
}

