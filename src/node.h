/* node.h - Node storage
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

#ifndef REDPILE_NODE_H
#define REDPILE_NODE_H

#include "location.h"
#include "common.h"
#include "message.h"
#include "type.h"

typedef int FieldValue;
typedef struct {
    unsigned int count;
    FieldValue data[];
} FieldData;

typedef struct Node {
    Location location;
    Type* type;

    // We keep references to the 6 nodes adjacent to this one for faster
    // access during redstone ticks.  This adds a bit of extra time to
    // insertions but more than makes up for it when running ticks
    struct Node* adjacent[6];
    struct Node* next;
    struct Node* prev;

    MessageStore* store;

    Messages* last_input;
    unsigned long long last_input_tick;

    FieldData fields;
} Node;

typedef struct {
    Node* active;
    Node* inactive;
    unsigned int size;
} NodeList;

typedef struct {
    int index;
    unsigned int count;
    Node* nodes[];
} NodeStack;

#define MAX_FIELDS 256
#define FIELD_GET(NODE,INDEX) (((INDEX) < (NODE)->fields.count) ? (NODE)->fields.data[INDEX] : 0)
#define FIELD_SET(NODE,INDEX,VALUE) if ((INDEX) < (NODE)->fields.count) { (NODE)->fields.data[INDEX] = VALUE; }
#define FOR_NODES(NODE,START) for (Node* NODE = START; NODE != NULL; NODE = NODE->next)

Messages* node_find_messages(Node* node, unsigned long long tick);
MessageStore* node_find_store(Node* node, unsigned long long tick);
void node_print(Node* node);

NodeList* node_list_allocate(void);
void node_list_free(NodeList* nodes);
Node* node_list_append(NodeList* nodes, Location location, Type* type);
void node_list_remove(NodeList* nodes, Node* node);
void node_list_move_after(NodeList* nodes, Node* node, Node* target);
void node_list_print(NodeList* nodes);

NodeStack* node_stack_allocate(unsigned int count);
void node_stack_free(NodeStack* stack);
int node_stack_push(NodeStack* stack, Node* node);
bool node_stack_pop(NodeStack* stack);
Node* node_stack_index(NodeStack* stack, unsigned int index);
Node* node_stack_first(NodeStack* stack);

#endif
