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
#include "redpile.h"
#include "message.h"

#define MATERIALS_COUNT 10
extern char* Materials[MATERIALS_COUNT];
extern unsigned int FieldCounts[MATERIALS_COUNT];

// The ordering of these matter
typedef enum {
    EMPTY,
    AIR,
    INSULATOR,
    WIRE,
    CONDUCTOR,
    TORCH,
    PISTON,
    REPEATER,
    COMPARATOR,
    SWITCH
} Material;

typedef struct {
    unsigned int count;
    Field data[];
} Fields;

typedef struct Node {
    Location location;
    Type type;

    // We keep references to the 6 blocks adjacent to this one for faster
    // access during redstone ticks.  This adds a bit of extra time to
    // insertions but more than makes up for it when running ticks
    struct Node* adjacent[6];
    struct Node* next;
    struct Node* prev;

    MessageStore* store;
    Fields fields;
} Node;

typedef struct {
    Node* nodes;
    unsigned int size;
} NodeList;

#define FIELD_GET(NODE,INDEX) (((INDEX) < (NODE)->fields.count) ? (NODE)->fields.data[INDEX] : 0)
#define FIELD_SET(NODE,INDEX,VALUE) if ((INDEX) < (NODE)->fields.count) { (NODE)->fields.data[INDEX] = VALUE; }
#define FOR_NODE_LIST(LIST) for (Node* node = LIST->nodes; node != NULL; node = node->next)

Messages* node_find_messages(Node* node, unsigned long long tick);
MessageStore* node_find_store(Node* node, unsigned long long tick);
void node_print(Node* node);
void node_print_power(Node* node);

NodeList* node_list_allocate(void);
void node_list_free(NodeList* blocks);
Node* node_list_append(NodeList* blocks, Location location, Type type);
void node_list_remove(NodeList* blocks, Node* node);
void node_list_move_after(NodeList* blocks, Node* node, Node* target);
void node_list_print(NodeList* blocks);

#endif
