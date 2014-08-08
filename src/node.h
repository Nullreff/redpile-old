/* node.h - Node storage
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
