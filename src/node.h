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

#ifndef REDPILE_BLOCK_H
#define REDPILE_BLOCK_H

#include "location.h"
#include "redpile.h"

#define MATERIALS_COUNT 10
#define MATERIAL_DEFAULT EMPTY
extern char* Materials[MATERIALS_COUNT];

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

unsigned int FieldCounts[MATERIALS_COUNT] = {0, 0, 0, 1, 1, 2, 2, 3, 3, 3};

typedef unsigned int Type;
typedef int Field;
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

    // True if this block was added by the system
    // False if it was added via command
    bool system:1;

    Fields fields;
} Node;

typedef struct {
    Node* nodes;
    unsigned int size;
} NodeList;

#define FIELD_GET(NODE,INDEX) (((INDEX) < (NODE)->fields.count) ? (NODE)->fields.data[INDEX] : 0)
#define FIELD_SET(NODE,INDEX,VALUE) (assert((INDEX) < (NODE)->fields.count), (NODE)->fields.data[INDEX] = VALUE)
#define FOR_NODE_LIST(LIST) for (Node* node = LIST->nodes; node != NULL; node = node->next)

int material_parse(char* material, Material* result);

void node_print(Node* node);
void node_print_power(Node* node);

NodeList* node_list_allocate(void);
void node_list_free(NodeList* blocks);
Node* node_list_append(NodeList* blocks, Location location, Type type, bool system);
void node_list_remove(NodeList* blocks, Node* node);
void node_list_move_after(NodeList* blocks, Node* node, Node* target);
void node_list_print(NodeList* blocks);

#endif
