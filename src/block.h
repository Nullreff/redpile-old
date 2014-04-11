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

#define POWER_SOURCE(material) (material == M_TORCH)

#define MATERIALS_COUNT 6
char* Materials[MATERIALS_COUNT];
typedef enum {
    M_EMPTY,
    M_AIR,
    M_WIRE,
    M_CONDUCTOR,
    M_INSULATOR,
    M_TORCH
} Material;

typedef enum {
    D_NORTH,
    D_SOUTH,
    D_EAST,
    D_WEST,
    D_UP,
    D_DOWN
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
    unsigned int power:4; // 0 - 15
    unsigned int updated:1;
} Block;

int material_parse(char* material, Material* result);
Location location_move(Location loc, Direction dir, int length);
int location_equals(Location l1, Location l2);
int location_hash(Location loc, int max);

#endif
