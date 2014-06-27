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

#define LOWER_POWER(NODE,POWER) messages_power_check(in, LOCATION(NODE), POWER)
#define NODE_ADJACENT(NODE,DIR) world_get_adjacent_node(world, NODE, DIR)
#define MOVE_TO_NODE(NODE,DIR) NODE = NODE_ADJACENT(NODE, DIR)

#define SEND_POWER(NODE,POWER,DELAY) queue_add(messages, MESSAGE_POWER,  world->ticks + (DELAY), node, NODE, POWER)
#define SEND_MOVE(NODE,DIR,DELAY)    queue_add(messages, MESSAGE_MOVE,   world->ticks + (DELAY), node, NODE, DIR  )
#define CMD_POWER(POWER)             queue_add(sets,     MESSAGE_POWER,  world->ticks,           node, node, POWER)
#define CMD_MOVE(DIR)                queue_add(sets,     MESSAGE_MOVE,   world->ticks,           node, node, DIR  )
#define CMD_REMOVE()                 queue_add(sets,     MESSAGE_REMOVE, world->ticks,           node, node, 0    )

#define TYPE_METHOD(TYPE)\
    static void redstone_ ## TYPE ## _update(World* world, Node* node, Messages* in, Queue* messages, Queue* sets)

#define TYPE_REGISTER(TYPE)\
    case TYPE: redstone_ ## TYPE ## _update(world, node, in, &output, &sets); break

TYPE_METHOD(EMPTY) {}
TYPE_METHOD(AIR) {}
TYPE_METHOD(INSULATOR) {}

TYPE_METHOD(CONDUCTOR)
{
    Message* move_inst = messages_find_move(in);
    if (move_inst != NULL)
    {
        CMD_MOVE(move_inst->message);
        return;
    }

    unsigned int power = messages_max_power(in);
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

TYPE_METHOD(WIRE)
{
    Message* move_inst = messages_find_move(in);
    if (move_inst != NULL)
    {
        CMD_REMOVE();
        return;
    }

    Node* above = NODE_ADJACENT(node, UP);
    bool covered = above != NULL && MATERIAL(above) != AIR && MATERIAL(above) != EMPTY;

    int new_power = messages_max_power(in);
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
TYPE_METHOD(PISTON)
{
    Node* first = NODE_ADJACENT(node, DIRECTION(node));
    Node* second = NODE_ADJACENT(first, DIRECTION(node));

    unsigned int new_power = messages_max_power(in);

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

    Message* move_inst = messages_find_move(in);
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

TYPE_METHOD(COMPARATOR)
{
    Message* move_inst = messages_find_move(in);
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
        Message* inst = in->data + i;

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

TYPE_METHOD(REPEATER)
{
    Message* move_inst = messages_find_move(in);
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
        Message* inst = in->data + i;

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

TYPE_METHOD(TORCH)
{
    Message* move_inst = messages_find_move(in);
    if (move_inst != NULL)
    {
        CMD_REMOVE();
        return;
    }

    unsigned int new_power = 0;
    Location loc_behind = location_move(LOCATION(node), direction_invert(DIRECTION(node)), 1);
    for (int i = 0; i < in->size; i++)
    {
        Message* inst = in->data + i;

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

TYPE_METHOD(SWITCH)
{
    Message* move_inst = messages_find_move(in);
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

static Message message_create(QueueData* data)
{
    return (Message){{data->source.location, data->source.type}, data->type, data->message};
}

static Messages* find_input(World* world, Node* node, Queue* queue)
{
    QueueNode* found = queue_find_nodes(queue, node, world->ticks);
    unsigned int new_messages = 0;
    QueueNode* iter = found;
    while (iter != NULL &&
           iter->data.tick == world->ticks &&
           iter->data.target.node == node)
    {
        new_messages++;
        iter = iter->next;
    }

    Messages* found_messages = node_find_messages(node, world->ticks);
    unsigned int total = (found_messages != NULL ? found_messages->size : 0) + new_messages;

    Messages* messages = messages_allocate(total);
    if (total == 0)
        return messages;

    for (int i = 0; i < new_messages; i++)
    {
        messages->data[i] = message_create(&found->data);
        found = found->next;
    }

    if (found_messages != NULL)
        messages_copy(messages->data + new_messages, found_messages);

    if (world->max_inputs < total)
        world->max_inputs = total;

    return messages;
}

static void process_output(World* world, Node* node, Queue* output, Queue* messages_out, Queue* sets_out)
{
    FOR_QUEUE(queue_node, output)
    {
        QueueData* data = &queue_node->data;

        if (data->tick == world->ticks && !queue_contains(messages_out, queue_node))
        {
            assert(!LOCATION_EQUALS(data->target.location, data->source.location));
            queue_remove_source(messages_out, data->target.location);
            queue_remove_source(sets_out, data->target.location);
            node_list_move_after(world->nodes, node, data->target.node);
        }
    }

    unsigned int count = queue_merge(messages_out, output);
    if (world->max_outputs < count)
        world->max_outputs = count;
}

static void run_messages(World* world, Queue* queue)
{
    FOR_QUEUE(queue_node, queue)
    {
        QueueData* data = &queue_node->data;

        assert(!LOCATION_EQUALS(data->target.location, data->source.location));
        Node* target = data->target.node;
        MessageStore* store = node_find_store(data->target.node, data->tick);

        unsigned int count = 0;
        QueueNode* iter = queue_node;
        while (iter != NULL &&
               iter->data.tick == store->tick &&
               iter->data.target.node == target)
        {
            count++;
            iter = iter->next;
        }

        unsigned int old_size = store->messages->size;
        store->messages = messages_resize(store->messages, old_size + count);

        store->messages->data[old_size] = message_create(data);
        for (int i = 1; i < count; i++)
        {
            queue_node = queue_node->next;
            store->messages->data[old_size + i] = message_create(data);
        }

        if (world->max_queued < count)
            world->max_queued = count;
    }
}

static void run_sets(World* world, Queue* sets, LogLevel log_level)
{
    FOR_QUEUE(set, sets)
    {
        if (log_level != LOG_QUIET)
            queue_data_print(&set->data, message_type_print);
        world_run_data(world, &set->data);
    }
}

void redstone_tick(World* world, unsigned int count, LogLevel log_level)
{
    world_set_node_missing_callback(world, redstone_node_missing);

    for (unsigned int i = 0; i < count; i++)
    {
        if (log_level == LOG_VERBOSE)
            printf("--- Tick %llu ---\n", world->ticks);

        unsigned int loops = 0;
        Queue messages = queue_empty(true, true, world->hashmap->size);
        Queue sets = queue_empty(false, true, world->hashmap->size);

        if (log_level == LOG_VERBOSE)
            printf("Nodes:\n");

        FOR_NODE_LIST(world->nodes)
        {
            if (log_level == LOG_VERBOSE)
                node_print(node);

            Messages* in = find_input(world, node, &messages);
            Queue output = queue_empty(false, false, 0);

            switch (MATERIAL(node))
            {
                TYPE_REGISTER(EMPTY);
                TYPE_REGISTER(AIR);
                TYPE_REGISTER(INSULATOR);
                TYPE_REGISTER(WIRE);
                TYPE_REGISTER(CONDUCTOR);
                TYPE_REGISTER(TORCH);
                TYPE_REGISTER(PISTON);
                TYPE_REGISTER(REPEATER);
                TYPE_REGISTER(COMPARATOR);
                TYPE_REGISTER(SWITCH);
                default: ERROR("Encountered unknown block material");
            }
            free(in);

            process_output(world, node, &output, &messages, &sets);

            loops++;
            if (loops > world->nodes->size * 2)
            {
                fprintf(stderr, "Logic loop detected while performing tick\n");
                break;
            }
        }

        if (log_level == LOG_VERBOSE)
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

