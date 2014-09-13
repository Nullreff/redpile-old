/* tick.c - Implementation of the TICK command
 *
 * Copyright (C) 2014 Ryan Mendivil <ryan@nullreff.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redpile nor the names of its contributors may be
 *     used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "tick.h"
#include "repl.h"

static Message message_create(QueueData* data)
{
    return (Message){{data->source.location, data->source.type}, data->type, data->message};
}

static Messages* find_input(World* world, Node* node, Queue* queue)
{
    QueueNode* new_messages;
    unsigned int new_size = 0;
    queue_find_nodes(queue, node, world->ticks, &new_messages, &new_size);

    Messages* existing_messages = node_find_messages(node, world->ticks);
    unsigned int existing_size = existing_messages != NULL ? existing_messages->size : 0;
    unsigned int total = new_size + existing_size;

    Messages* messages = messages_allocate(total);
    if (total == 0)
        return messages;

    if (existing_size != 0)
        messages_copy(messages->data, existing_messages);

    if (new_size != 0)
    {
        QueueNode* found = new_messages;
        for (int i = existing_size; i < total; i++)
        {
            messages->data[i] = message_create(&found->data);
            found = found->next;
            if (found == NULL || found->data.tick != world->ticks || found->data.target.node != node)
            {
                messages->size = i + 1;
                break;
            }
        }
    }

    if (world->max_inputs < messages->size)
        world->max_inputs = messages->size;

    return messages;
}

#define PROCESS_DONE 0
#define PROCESS_CHANGED 1
#define PROCESS_ERROR 2
static int process_node(ScriptState* state, World* world, Node* node, Queue* output, Queue* messages, Queue* sets)
{
    Messages* input = find_input(world, node, messages);

    for (int i = 0; i < node->type->behaviors->count; i++)
    {
        Behavior* behavior = node->type->behaviors->data[i];
        Messages* found = messages_filter_copy(input, behavior->mask);
        ScriptData data = (ScriptData){world, node, found, output, sets};
        bool success = script_state_run_behavior(state, behavior, &data);
        free(found);
        if (!success)
            return PROCESS_ERROR;
    }

    if (node->last_input == NULL)
    {
        node->last_input = input;
        node->last_input_tick = world->ticks;
        return PROCESS_CHANGED;
    }
    else if (node->last_input_tick != world->ticks ||
            !messages_equal(node->last_input, input))
    {
        free(node->last_input);
        node->last_input = input;
        node->last_input_tick = world->ticks;
        return PROCESS_CHANGED;
    }
    else
    {
        free(input);
        return PROCESS_DONE;
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
        if (world_run_data(world, &set->data) && log_level != LOG_QUIET)
            queue_data_print(&set->data);
    }
}

void tick_run(ScriptState* state, World* world, unsigned int count, LogLevel log_level)
{
    world_set_node_missing_callback(world, true);

    for (int i = 0; i < count; i++)
    {
        if (log_level == LOG_VERBOSE)
            repl_print("--- Tick %llu ---\n", world->ticks);

        unsigned long long loops = 0;
        Queue messages = queue_empty(true, true, world->hashmap->size);
        Queue sets = queue_empty(false, true, world->hashmap->size);

        if (log_level == LOG_VERBOSE)
            repl_print("Nodes:\n");

        FOR_NODES(node, world->nodes->active)
        {
            if (log_level == LOG_VERBOSE)
                node_print(node);

            Queue output = queue_empty(false, false, 0);
            bool status = process_node(state, world, node, &output, &messages, &sets);
            if (status == PROCESS_ERROR)
                return;

            process_output(world, node, status == PROCESS_CHANGED, &output, &messages, &sets);

            loops++;
            if (loops > world->nodes->size * 3)
            {
                repl_print_error("Logic loop detected while performing tick\n");
                break;
            }
        }

        if (log_level == LOG_VERBOSE)
        {
            repl_print("Messages:\n");
            FOR_QUEUE(message, &messages)
            {
                if (message->data.tick == world->ticks)
                    queue_data_print_verbose(&message->data, world->ticks);
            }

            repl_print("Queued:\n");
            FOR_QUEUE(message, &messages)
            {
                if (message->data.tick > world->ticks)
                    queue_data_print_verbose(&message->data, world->ticks);
            }
            repl_print("Output:\n");
        }

        run_messages(world, &messages);
        run_sets(world, &sets, log_level);
        
        queue_free(&messages);
        queue_free(&sets);
        world->ticks++;
    }

    world_set_node_missing_callback(world, false);
}

