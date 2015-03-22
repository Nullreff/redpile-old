/* queue.h - Message queue with fast search by source/target
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
    Node target;
    unsigned long long tick;
    unsigned int type;
    unsigned int index;
    FieldValue value;
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
    Hashmap targetmap;
    Hashmap sourcemap;
} Queue;

#define FOR_QUEUE(NODE,QUEUE) for (QueueNode* (NODE) = (QUEUE)->nodes; (NODE) != NULL; (NODE) = (NODE)->next)

void queue_init(Queue* queue, bool track_targets, bool track_sources, unsigned int size);
void queue_free(Queue* queue);
void queue_add(Queue* queue, unsigned int type, unsigned long long tick, Node* source, Node* target, unsigned int index, FieldValue value);
bool queue_contains(Queue* queue, QueueNode* node);
unsigned int queue_merge(Queue* queue, Queue* append);
void queue_remove_source(Queue* queue, Location source);
void queue_find_nodes(Queue* messages, Node* target, unsigned long long tick, QueueNode** found_node, unsigned int* max_size);
void queue_data_print(QueueData* data);
void queue_data_print_message(QueueData* data, TypeData* type_data, unsigned long long current_tick);

#endif

