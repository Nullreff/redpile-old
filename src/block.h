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

#define MATERIALS_COUNT 6
#define MATERIAL_DEFAULT EMPTY
char* Materials[MATERIALS_COUNT];
typedef enum {
    EMPTY,
    AIR,
    WIRE,
    CONDUCTOR,
    INSULATOR,
    TORCH
} Material;

typedef struct {
    Location location;
    Material material;
    Direction direction;
    unsigned int power:4; // 0 - 15
    unsigned int updated:1;
} Block;

typedef struct {
    Block* data;
    unsigned int index;
    unsigned int size;
} BlockList;

#define POWER_SOURCE(material) (material == TORCH)
#define HAS_DIRECTION(material) (material == TORCH)
int material_parse(char* material, Material* result);

Block block_empty(void);
Block block_from_values(int values[]);
Block block_create(Location location, Material material, Direction direction);
void block_allocate(Block** block, Location location, Material material, Direction direction);
void block_copy(Block* dest, Block* source);
void block_print(Block* block);
void block_print_power(Block* block);

BlockList* block_list_allocate(unsigned int size);
void block_list_free(BlockList* blocks);
int block_list_next(BlockList* blocks);

#endif
