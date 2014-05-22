/* block.h - Blocks and block related data structures
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

#define MATERIALS_COUNT 9
#define MATERIAL_DEFAULT EMPTY
extern char* Materials[MATERIALS_COUNT];
typedef enum {
    EMPTY,
    AIR,
    WIRE,
    CONDUCTOR,
    INSULATOR,
    TORCH,
    REPEATER,
    COMPARATOR,
    PISTON
} Material;

typedef struct {
    // General information
    Location location;
    Material material;
    Direction direction;
    unsigned int state;

    // Redstone state
    unsigned int power;
    unsigned int power_state;
    unsigned int last_power;
    bool updated;

    // True if this block was added by the system
    // False if it was added via command
    bool system;
} Block;

typedef struct BlockNode {
    Block block;

    // We keep references to the 6 blocks adjacent to this one for faster
    // access during redstone ticks.  This adds a bit of extra time to
    // insertions but more than makes up for it when running ticks
    struct BlockNode* adjacent[6];
    struct BlockNode* next;
    struct BlockNode* prev;
} BlockNode;

typedef struct {
    BlockNode* head;
    BlockNode* tail;

    // Stats
    unsigned int size;
    unsigned int power_sources;
} BlockList;

#define TICK_BOUNDARY(material) (material == TORCH || material == REPEATER || material == COMPARATOR || material == PISTON)
#define HAS_DIRECTION(material) (material == TORCH || material == REPEATER || material == COMPARATOR || material == PISTON)
#define HAS_STATE(material) (material == REPEATER || material == COMPARATOR || material == PISTON)

int material_parse(char* material, Material* result);

Block block_empty(void);
Block block_from_values(int values[]);
Block block_create(Location location, Material material, Direction direction, unsigned int state);
void block_move(Block* dest, Block* src);
void block_print(Block* block);
void block_print_power(Block* block);

BlockList* block_list_allocate(void);
void block_list_free(BlockList* blocks);
BlockNode* block_list_append(BlockList* blocks, Block* block);
void block_list_remove(BlockList* blocks, BlockNode* node);

#endif
