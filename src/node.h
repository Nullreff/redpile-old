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

typedef struct {
    // General information
    Material material;
    Direction direction:3;
    unsigned int state:2;
    unsigned int power:4;
} Block;

typedef struct Node {
    Location location;
    Block block;

    // We keep references to the 6 blocks adjacent to this one for faster
    // access during redstone ticks.  This adds a bit of extra time to
    // insertions but more than makes up for it when running ticks
    struct Node* adjacent[6];
    struct Node* next;
    struct Node* prev;

    // True if this block was added by the system
    // False if it was added via command
    bool system:1;
} Node;

typedef struct {
    Node* nodes;
    unsigned int size;
} NodeList;

#define FOR_BLOCK_LIST(LIST) for (Node* node = LIST->nodes; node != NULL; node = node->next)

int material_parse(char* material, Material* result);

Block block_empty(void);
Block block_from_values(int values[]);
Block block_random(void);
Block block_create(Material material, Direction direction, unsigned int state);
void block_print(Block* block);
void node_print_power(Node* node);

NodeList* node_list_allocate(void);
void node_list_free(NodeList* blocks);
Node* node_list_append(NodeList* blocks, Location location, Block* block, bool system);
void node_list_remove(NodeList* blocks, Node* node);
void node_list_move_after(NodeList* blocks, Node* node, Node* target);
void node_list_print(NodeList* blocks);

#endif
