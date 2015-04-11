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
    return (Message){{data->source.location, data->source.data->type}, data->type, data->value.integer};
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
        for (unsigned int i = existing_size; i < total; i++)
        {
            messages->data[i] = message_create(&found->data);
            found = found->next;
            if (found == NULL || found->data.tick != world->ticks || !node_equals(&found->data.target, node))
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

static bool process_node(ScriptState* state, World* world, Node* node, Queue* output, Queue* messages)
{
    Messages* input = find_input(world, node, messages);
    NodeData* data = node->data;

    for (unsigned int i = 0; i < node->data->type->behaviors->count; i++)
    {
        Behavior* behavior = data->type->behaviors->data[i];
        Messages* found = messages_filter_copy(input, behavior->mask);
        ScriptData data = (ScriptData){world, node, found, output};
        bool success = script_state_run_behavior(state, behavior, &data);
        free(found);
        if (!success)
            return false;
    }

    free(input);
    return true;
}

static void process_output(World* world, Node* node, Queue* messages, Queue* output, Hashmap* rerun)
{
    if (world->max_outputs < output->count)
        world->max_outputs = output->count;

    // Find messages that were removed
    Bucket* bucket = hashmap_get(&messages->sourcemap, node->location, false);
    if (bucket != NULL)
    {
        QueueNodeList* node_list = bucket->value;
        int index = 0;
        for (unsigned int i = 0; i < node_list->size; i++)
        {
            QueueNode* queue_node = node_list->nodes[i];
            QueueNode* found = queue_find(output, queue_node);
            if (!found)
            {
                QueueData* data = &queue_node->data;
                if (data->tick == world->ticks &&
                    !location_equals(data->target.location, location_empty()))
                {
                    Bucket* bucket = hashmap_get(rerun, data->target.location, true);
                    bucket->value = data->target.data;
                }
                queue_remove(messages, queue_node);
            }
            else
            {
                queue_remove(output, found);
                node_list->nodes[index] = node_list->nodes[i];
                index++;
            }
        }

        node_list->size = index;
    }

    // Find messages that were added
    QueueNode* queue_node = output->nodes;
    while (queue_node != NULL)
    {
        QueueData* data = &queue_node->data;
        if (data->tick == world->ticks &&
            !location_equals(data->target.location, location_empty()))
        {
            Bucket* bucket = hashmap_get(rerun, data->target.location, true);
            bucket->value = data->target.data;
        }
        QueueNode* temp = queue_node;
        queue_node = queue_node->next;
        queue_push(messages, temp);
    }

    output->nodes = NULL;
}

static void run_messages(World* world, Queue* queue, LogLevel log_level)
{
    FOR_QUEUE(queue_node, queue)
    {
        QueueData* data = &queue_node->data;

        if (location_equals(data->target.location, location_empty()))
        {
            if (world_run_data(world, data) && log_level != LOG_QUIET)
                queue_data_print(data);
        }
        else
        {
            MessageStore* store = node_find_store(&data->target, data->tick);

            unsigned int count = 0;
            QueueNode* iter = queue_node;
            while (iter != NULL &&
                   iter->data.tick == store->tick &&
                   node_equals(&iter->data.target, &data->target))
            {
                count++;
                iter = iter->next;
            }

            unsigned int old_size = store->messages->size;
            store->messages = messages_resize(store->messages, old_size + count);

            store->messages->data[old_size] = message_create(data);
            for (unsigned int i = 1; i < count; i++)
            {
                queue_node = queue_node->next;
                store->messages->data[old_size + i] = message_create(data);
            }

            if (world->max_queued < count)
                world->max_queued = count;
        }
    }
}

void tick_run(ScriptState* state, World* world, unsigned int count, LogLevel log_level)
{
    for (unsigned int i = 0; i < count; i++)
    {
        if (log_level == LOG_VERBOSE)
            repl_print("=== Tick %llu ===\n", world->ticks);

        Queue messages;
        queue_init(&messages, true, true, 1024);

        Hashmap* run = &world->nodes;
        Hashmap* rerun = malloc(sizeof(Hashmap));
        CHECK_OOM(rerun);
        hashmap_init(rerun, run->size);

        unsigned int iterations = 0;
        while (run->count > 0)
        {
            if (log_level == LOG_VERBOSE)
                repl_print("--- Pass %d (%d nodes)---\n", iterations, run->count);

            Cursor cursor = hashmap_get_iterator(run);
            Node node;

            while (cursor_next(&cursor, &node.location, (void**)&node.data))
            {
                if (log_level == LOG_VERBOSE)
                    node_print(&node);

                Queue output;
                queue_init(&output, true, false, 1024);
                bool status = process_node(state, world, &node, &output, &messages);
                if (!status)
                    return;

                process_output(world, &node, &messages, &output, rerun);
                queue_free(&output);
            }

            if (run != &world->nodes)
            {
                hashmap_free(run, NULL);
                free(run);
            }

            run = rerun;
            rerun = malloc(sizeof(Hashmap));
            CHECK_OOM(rerun);
            hashmap_init(rerun, run->size);
            
            if (iterations > 16)
            {
                repl_print_error("Logic loop detected while performing tick\n");
                break;
            }
            iterations++;
        }

        if (run != &world->nodes)
        {
            hashmap_free(run, NULL);
            free(run);
        }
        hashmap_free(rerun, NULL);
        free(rerun);

        if (log_level == LOG_VERBOSE)
        {
            repl_print("Messages:\n");
            FOR_QUEUE(message, &messages)
            {
                if (message->data.tick == world->ticks &&
                    !location_equals(message->data.target.location, location_empty()))
                {
                    queue_data_print_message(&message->data, world->type_data, world->ticks);
                }
            }

            repl_print("Queued:\n");
            FOR_QUEUE(message, &messages)
            {
                if (message->data.tick > world->ticks)
                    queue_data_print_message(&message->data, world->type_data, world->ticks);
            }
            repl_print("Output:\n");
        }

        run_messages(world, &messages, log_level);
        
        queue_free(&messages);
        world_gc_nodes(world);
        world->ticks++;
    }
}

