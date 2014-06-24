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

typedef struct QueueNode {
    Location source;
    Location target;
    void* value;
    struct QueueNode* next;
    struct QueueNode* prev;
} QueueNode;

typedef struct {
    unsigned int size;
    QueueNode* nodes[];
} QueueNodeList;

typedef struct {
    QueueNode* nodes;
    Hashmap* targetmap;
    Hashmap* sourcemap;
} Queue;

#define FOR_QUEUE(NODE,QUEUE) for (QueueNode* (NODE) = (QUEUE)->nodes; (NODE) != NULL; (NODE) = (NODE)->next)

Queue queue_empty(bool track_targets, bool track_sources, unsigned int size);
void queue_free(Queue* queue);
QueueNode* queue_add(Queue* queue, Location source, Location target);
void queue_push(Queue* queue, QueueNode* node);
bool queue_contains(Queue* queue, QueueNode* node, bool (*compare)(void* first, void* second));
void queue_merge(Queue* queue, Queue* append, bool (*compare)(void* first, void* second));
void queue_remove(Queue* queue, QueueNode* node);
void queue_remove_source(Queue* queue, Location source);

#endif

