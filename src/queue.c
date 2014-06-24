/* queue.c - Message queue with fast search by source/target
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

#include "queue.h"

static bool queue_data_equals(QueueData* n1, QueueData* n2)
{
    return n1->type == n2->type &&
           LOCATION_EQUALS(n1->source.location, n2->source.location) &&
           n1->target.node == n2->target.node &&
           n1->message == n2->message &&
           n1->tick == n2->tick;
}

static void queue_push(Queue* queue, QueueNode* node)
{
    if (queue->targetmap != NULL)
    {
        // Add the node to the list by it's target
        Bucket* bucket = hashmap_get(queue->targetmap, node->data.target.location, true);
        if (bucket->value == NULL)
        {
            // No nodes with this source exist yet
            // Stick it in the front of the list
            node->next = queue->nodes;
            node->prev = NULL;
            if (queue->nodes != NULL)
                queue->nodes->prev = node;
            queue->nodes = node;
            bucket->value = node;
        }
        else
        {
            // Otherwise stick it after the existing one
            QueueNode* existing = bucket->value;
            node->next = existing->next;
            node->prev = existing;
            if (existing->next != NULL)
                existing->next->prev = node;
            existing->next = node;
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

    if (queue->sourcemap != NULL)
    {
        // Add a reference to it by source
        Bucket* bucket = hashmap_get(queue->sourcemap, node->data.source.location, true);
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
}

static void queue_remove(Queue* queue, QueueNode* node)
{
    if (queue->targetmap != NULL)
    {
        Bucket* bucket = hashmap_get(queue->targetmap, node->data.target.location, false);
        assert(bucket != NULL);

        if (bucket->value == node)
        {
            if (node->next != NULL && node->data.target.node == node->next->data.target.node)
                // Change the hashmap to target the next node
                bucket->value = node->next;
            else
                // We ran out of nodes with this target
                bucket->value = NULL;
        }
    }

    if (node->prev != NULL)
        node->prev->next = node->next;
    else
        queue->nodes = node->next;

    if (node->next != NULL)
        node->next->prev = node->prev;

    free(node);
}

Queue queue_empty(bool track_targets, bool track_sources, unsigned int size)
{
    Hashmap* targetmap = track_targets ? hashmap_allocate(size) : NULL;
    Hashmap* sourcemap = track_sources ? hashmap_allocate(size) : NULL;
    return (Queue){NULL, targetmap, sourcemap};
}

void queue_free(Queue* queue)
{
    QueueNode* node = queue->nodes;
    while (node != NULL)
    {
        QueueNode* temp = node->next;
        free(node);
        node = temp;
    }

    if (queue->targetmap != NULL)
        hashmap_free(queue->targetmap, NULL);
    if (queue->sourcemap != NULL)
        hashmap_free(queue->sourcemap, free);
}

void queue_add(Queue* queue, unsigned int type, unsigned long long tick, Node* source, Node* target, unsigned int message)
{
    QueueNode* node = malloc(sizeof(QueueNode));
    node->data = (QueueData) {
        .source.location = source->location,
        .source.type = source->type,
        .target.location = target->location,
        .target.node = target,
        .tick = tick,
        .type = type,
        .message = message
    };
    queue_push(queue, node);
}

bool queue_contains(Queue* queue, QueueNode* node)
{
    Bucket* bucket = hashmap_get(queue->targetmap, node->data.target.location, false);
    if (bucket == NULL)
        return false;

    QueueNode* found = bucket->value;
    if (found == NULL)
        return false;

    do
    {
        if (queue_data_equals(&found->data, &node->data))
            return true;
        found = found->next;
    }
    while (found != NULL && found->data.target.node == node->data.target.node);

    return false;
}

void queue_merge(Queue* queue, Queue* append)
{
    // Exit early if theres nothing to append
    if (append->nodes == NULL)
        return;

    // Merge in any that haven't already been added
    QueueNode* node = append->nodes;
    while (node != NULL)
    {
        QueueNode* temp = node->next;
        if (!queue_contains(queue, node))
            queue_push(queue, node);
        else
            free(node);
        node = temp;
    }

    append->nodes = NULL;
}

void queue_remove_source(Queue* queue, Location source)
{
    Bucket* bucket = hashmap_get(queue->sourcemap, source, false);
    if (bucket == NULL)
        return;

    QueueNodeList* node_list = bucket->value;
    for (int i = 0; i < node_list->size; i++)
        queue_remove(queue, node_list->nodes[i]);

    node_list->size = 0;
}

QueueNode* queue_find_nodes(Queue* messages, Node* target, unsigned long long tick)
{
    Bucket* bucket = hashmap_get(messages->targetmap, target->location, false);
    if (bucket == NULL)
        return NULL;

    QueueNode* found = bucket->value;
    if (found == NULL)
        return NULL;

    do
    {
        if (found->data.tick == tick)
            return found;
        found = found->next;
    }
    while (found != NULL && found->data.target.node == target);

    return NULL;
}

void queue_data_print(
    QueueData* data,
    void (*print_message)(unsigned int type, unsigned int message))
{
    printf("(%d,%d,%d) ",
        data->target.location.x,
        data->target.location.y,
        data->target.location.z);

    print_message(data->type, data->message);
}


void queue_data_print_verbose(
    QueueData* data,
    void (*print_message)(unsigned int type, unsigned int message),
    unsigned long long current_tick)
{
    printf("%llu (%d,%d,%d) => (%d,%d,%d) ",
        data->tick - current_tick,
        data->source.location.x,
        data->source.location.y,
        data->source.location.z,
        data->target.location.x,
        data->target.location.y,
        data->target.location.z);

    print_message(data->type, data->message);
}
