/* queue.h - Message queue with fast search by source/target
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

#ifndef REDPILE_QUEUE_H
#define REDPILE_QUEUE_H

#include "location.h"
#include "hashmap.h"
#include "node.h"
#include "type.h"

typedef struct {
    struct {
        Location location;
        Type* type;
    } source;
    struct {
        Location location;
        Node* node;
    } target;
    unsigned long long tick;
    unsigned int type;
    int64_t message;
} QueueData;

typedef struct QueueNode {
    struct QueueNode* next;
    struct QueueNode* prev;
    QueueData data;
} QueueNode;

typedef struct {
    unsigned int size;
    QueueNode* nodes[];
} QueueNodeList;

typedef struct {
    unsigned int size;
    QueueNode* node;
} QueueNodeIndex;

typedef struct {
    QueueNode* nodes;
    Hashmap* targetmap;
    Hashmap* sourcemap;
} Queue;

#define FOR_QUEUE(NODE,QUEUE) for (QueueNode* (NODE) = (QUEUE)->nodes; (NODE) != NULL; (NODE) = (NODE)->next)

Queue queue_empty(bool track_targets, bool track_sources, unsigned int size);
void queue_free(Queue* queue);
void queue_add(Queue* queue, unsigned int type, unsigned long long tick, Node* source, Node* target, int64_t message);
bool queue_contains(Queue* queue, QueueNode* node);
unsigned int queue_merge(Queue* queue, Queue* append);
void queue_remove_source(Queue* queue, Location source);
void queue_find_nodes(Queue* messages, Node* target, unsigned long long tick, QueueNode** found_node, unsigned int* max_size);
void queue_data_print(QueueData* data, void (*print_message)(unsigned int type, int64_t message));
void queue_data_print_verbose(QueueData* data, void (*print_message)(unsigned int type, int64_t message), unsigned long long current_tick);

#endif

