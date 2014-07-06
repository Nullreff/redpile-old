/* tick.c - Implementation of the TICK command
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

#include "tick.h"
#include "redstone.h"

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

static bool process_node(World* world, Node* node, Queue* output, Queue* messages, Queue* sets)
{
    Messages* in = find_input(world, node, messages);

    switch (node->type)
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

    if (node->last_input == NULL)
    {
        node->last_input = in;
        node->last_input_tick = world->ticks;
        return true;
    }
    else if (node->last_input_tick != world->ticks ||
            !messages_equal(node->last_input, in))
    {
        free(node->last_input);
        node->last_input = in;
        node->last_input_tick = world->ticks;
        return true;
    }
    else
    {
        free(in);
        return false;
    }
}

static void process_output(World* world, Node* node, bool changed, Queue* output, Queue* messages, Queue* sets)
{
    if (changed)
    {
        FOR_QUEUE(queue_node, output)
        {
            QueueData* data = &queue_node->data;

            if (data->tick == world->ticks && !queue_contains(messages, queue_node))
            {
                assert(!LOCATION_EQUALS(data->target.location, data->source.location));
                queue_remove_source(messages, data->target.location);
                queue_remove_source(sets, data->target.location);
                node_list_move_after(world->nodes, node, data->target.node);
            }
        }
    }

    unsigned int count = queue_merge(messages, output);
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

void tick_run(World* world, unsigned int count, LogLevel log_level)
{
    world_set_node_missing_callback(world, redstone_node_missing);

    for (int i = 0; i < count; i++)
    {
        if (log_level == LOG_VERBOSE)
            printf("--- Tick %llu ---\n", world->ticks);

        unsigned long long loops = 0;
        Queue messages = queue_empty(true, true, world->hashmap->size);
        Queue sets = queue_empty(false, true, world->hashmap->size);

        if (log_level == LOG_VERBOSE)
            printf("Nodes:\n");

        FOR_NODE_LIST(node, world->nodes)
        {
            if (log_level == LOG_VERBOSE)
                node_print(node);

            Queue output = queue_empty(false, false, 0);
            bool changed = process_node(world, node, &output, &messages, &sets);
            process_output(world, node, changed, &output, &messages, &sets);

            loops++;
            if (loops > world->nodes->size * 3)
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

