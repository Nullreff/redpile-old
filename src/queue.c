/* queue.c - Message queue with fast search by source/target
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

#include "queue.h"
#include "repl.h"
#include <inttypes.h>

static bool queue_data_equals(QueueData* n1, QueueData* n2)
{
    return n1->type == n2->type &&
           location_equals(n1->source.location, n2->source.location) &&
           node_equals(&n1->target, &n2->target) &&
           n1->value.integer == n2->value.integer &&
           n1->tick == n2->tick;
}

void queue_init(Queue* queue, bool track_targets, bool track_sources, unsigned int size)
{
    queue->nodes = NULL;
    queue->count = 0;
    hashmap_init(&queue->targetmap, track_targets ? size : 0);
    hashmap_init(&queue->sourcemap, track_sources ? size : 0);
}

void queue_free(Queue* queue)
{
    QueueNode* node = queue->nodes;
    while (node != NULL)
    {
        QueueNode* temp = node->next;
        if (node->data.type == SM_DATA)
            free(node->data.value.string);
        free(node);
        node = temp;
    }

    hashmap_free(&queue->targetmap, free);
    hashmap_free(&queue->sourcemap, free);
}

void queue_push(Queue* queue, QueueNode* node)
{
    if (queue->targetmap.size > 0)
    {
        // Add the node to the list by it's target
        Bucket* bucket = hashmap_get(&queue->targetmap, node->data.target.location, true);
        QueueNodeIndex* index;
        if (bucket->value == NULL)
        {
            index = malloc(sizeof(QueueNodeIndex));
            index->size = 0;
            index->node = NULL;
            bucket->value = index;
        }
        else
        {
            index = bucket->value;
        }

        if (index->size == 0)
        {
            // No nodes with this source exist yet
            // Stick it in the front of the list
            node->next = queue->nodes;
            node->prev = NULL;
            if (queue->nodes != NULL)
                queue->nodes->prev = node;
            queue->nodes = node;

            index->node = node;
            index->size++;
        }
        else
        {
            // Otherwise stick it after the existing one
            QueueNodeIndex* index = bucket->value;
            QueueNode* existing = index->node;
            node->next = existing->next;
            node->prev = existing;
            if (existing->next != NULL)
                existing->next->prev = node;
            existing->next = node;
            index->size++;
        }
    }
    else
    {
        node->next = queue->nodes;
        node->prev = NULL;
        if (queue->nodes != NULL)
            queue->nodes->prev = node;
        queue->nodes = node;
    }

    if (queue->sourcemap.size > 0)
    {
        // Add a reference to it by source
        Bucket* bucket = hashmap_get(&queue->sourcemap, node->data.source.location, true);
        if (bucket->value == NULL)
        {
            QueueNodeList* source_list = malloc(sizeof(QueueNodeList) +  sizeof(QueueNode));
            CHECK_OOM(source_list);
            source_list->size = 1;
            source_list->nodes[0] = node;
            bucket->value = source_list;
        }
        else
        {
            QueueNodeList* source_list = bucket->value;
            unsigned int new_size = source_list->size + 1;

            // TODO: Pre-allocate space instead of reallocating on each add
            source_list = realloc(source_list, sizeof(QueueNodeList) + (sizeof(QueueNode) * new_size));
            CHECK_OOM(source_list);
            source_list->size = new_size;
            source_list->nodes[new_size - 1] = node;
            bucket->value = source_list;
        }
    }

    queue->count++;
}

void queue_remove(Queue* queue, QueueNode* node)
{
    if (queue->targetmap.size > 0)
    {
        Bucket* bucket = hashmap_get(&queue->targetmap, node->data.target.location, false);
        assert(bucket != NULL);

        QueueNodeIndex* index = bucket->value;
        index->size--;
        if (index->node == node)
        {
            if (index->size > 0)
            {
                // Change the hashmap to target the next node
                assert(node->next != NULL && node_equals(&node->data.target, &node->next->data.target));
                index->node = node->next;
            }
            else
            {
                // We ran out of nodes with this target
                assert(node->next == NULL || !node_equals(&node->data.target, &node->next->data.target));
                index->node = NULL;
            }
        }
    }

    if (node->prev != NULL)
        node->prev->next = node->next;
    else
        queue->nodes = node->next;

    if (node->next != NULL)
        node->next->prev = node->prev;

    free(node);
    queue->count--;
}

void queue_add_system(Queue* queue, unsigned int type, Node* source, unsigned int index, FieldValue value)
{
    QueueNode* node = malloc(sizeof(QueueNode));
    node->data = (QueueData) {
        .source = *source,
        .target = node_empty(),
        .tick = 0,
        .type = type,
        .index = index,
        .value = value
    };
    queue_push(queue, node);
}

void queue_add_message(Queue* queue, unsigned int type, unsigned long long tick, Node* source, Node* target, FieldValue value)
{
    QueueNode* node = malloc(sizeof(QueueNode));
    node->data = (QueueData) {
        .source = *source,
        .target = *target,
        .tick = tick,
        .type = type,
        .index = 0,
        .value = value
    };
    queue_push(queue, node);
}

QueueNode* queue_find(Queue* queue, QueueNode* node)
{
    Bucket* bucket = hashmap_get(&queue->targetmap, node->data.target.location, false);
    if (bucket == NULL)
        return NULL;

    QueueNodeIndex* index = bucket->value;
    QueueNode* found = index->node;
    if (found == NULL)
        return NULL;

    for (unsigned int i = 0; i < index->size; i++)
    {
        assert(found != NULL && node_equals(&found->data.target, &node->data.target));
        if (queue_data_equals(&found->data, &node->data))
            return found;
        found = found->next;
    }

    return NULL;
}

void queue_find_nodes(Queue* messages, Node* target, unsigned long long tick, QueueNode** found_node, unsigned int* max_size)
{
    Bucket* bucket = hashmap_get(&messages->targetmap, target->location, false);
    if (bucket == NULL)
        goto end;

    QueueNodeIndex* index = bucket->value;
    QueueNode* found = index->node;
    if (found == NULL)
        goto end;

    for (unsigned int i = 0; i < index->size; i++)
    {
        assert(found != NULL && node_equals(&found->data.target, target));
        if (found->data.tick == tick)
        {
            *found_node = found;
            *max_size = index->size - i;
            return;
        }
        found = found->next;
    }
    assert(found == NULL || !node_equals(&found->data.target, target));

end:
    *found_node = NULL;
    *max_size = 0;
    return;
}

static void queue_data_print_type(QueueData* data)
{
    switch (data->type)
    {
        case SM_MOVE:   repl_print("MOVE %s\n", Directions[data->value.direction]); break;
        case SM_REMOVE: repl_print("REMOVE\n"); break;
        case SM_DATA:
            repl_print("DATA \"%s\"\n", data->value.string ? data->value.string : "");
            break;
        case SM_FIELD:
            repl_print("FIELD");
            node_print_field(
                data->source.data->type->fields->data + data->index,
                data->value);
            repl_print("\n");
            break;
    }
}

void queue_data_print(QueueData* data)
{
    repl_print("%d,%d,%d ",
        data->source.location.x,
        data->source.location.y,
        data->source.location.z);

    queue_data_print_type(data);
}

void queue_data_print_message(QueueData* data, TypeData* type_data, unsigned long long current_tick)
{
    repl_print("%llu %d,%d,%d => %d,%d,%d ",
        data->tick - current_tick,
        data->source.location.x,
        data->source.location.y,
        data->source.location.z,
        data->target.location.x,
        data->target.location.y,
        data->target.location.z);

    for (MessageType* mt = type_data->message_types; mt != NULL; mt = mt->next)
    {
        if (mt->id == data->type)
        {
            // TODO: Print additional types
            repl_print("%s %d\n", mt->name, data->value.integer);
            break;
        }
    }
}

