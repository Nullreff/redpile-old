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

// I chose 101 because it kind of looks like two redstone torches
// If you have a better prime number, feel free to use it :)
#define MAGIC_HASH_NUMBER 101
#define MATERIALS_COUNT 6
#define DIRECTIONS_COUNT 6
#define POWER_SOURCE(material) (material == TORCH)

char* Materials[MATERIALS_COUNT];
typedef enum {
    EMPTY,
    AIR,
    WIRE,
    CONDUCTOR,
    INSULATOR,
    TORCH
} Material;

char* Directions[DIRECTIONS_COUNT];
typedef enum {
    NORTH,
    SOUTH,
    EAST,
    WEST,
    UP,
    DOWN
} Direction;

typedef int Coord;
typedef struct {
    Coord x;
    Coord y;
    Coord z;
} Location;

typedef struct {
    Material material;
    Location location;
    Direction direction;
    unsigned int power:4; // 0 - 15
    unsigned int updated:1;
} Block;

int material_parse(char* material, Material* result);
int material_has_direction(Material material);

int direction_parse(char* direction, Direction* result);
Direction direction_invert(Direction dir);

Location location_empty(void);
Location location_from_values(int values[]);
Location location_create(Coord x, Coord y, Coord z);
Location location_move(Location loc, Direction dir, int length);
int location_equals(Location l1, Location l2);
int location_hash(Location loc, int max);

Block block_empty(void);
Block block_from_values(int values[]);
Block block_create(Location location, Material material, Direction direction);
void block_allocate(Block** block, Location location, Material material, Direction direction);
void block_print(Block* block);

#endif
