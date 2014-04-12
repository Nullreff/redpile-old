/* block.c - Blocks and block related data structures
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "block.h"

char* Materials[MATERIALS_COUNT] = {
    "EMPTY",
    "AIR",
    "WIRE",
    "CONDUCTOR",
    "INSULATOR",
    "TORCH"
};

char* Directions[6] = {
    "NORTH",
    "SOUTH",
    "EAST",
    "WEST",
    "UP",
    "DOWN"
};

int material_parse(char* material, Material* result)
{
    int i;
    for (i = 0; i < MATERIALS_COUNT; i++)
    {
        if (strcmp(material, Materials[i]) == 0)
        {
            *result = (Material)i;
            return 0;
        }
    }

    return -1;
}

int material_has_direction(Material material)
{
    return material == TORCH;
}

int direction_parse(char* direction, Direction* result)
{
    int i;
    for (i = 0; i < DIRECTIONS_COUNT; i++)
    {
        if (strcmp(direction, Directions[i]) == 0)
        {
            *result = (Direction)i;
            return 0;
        }
    }

    return -1;
}

Direction direction_invert(Direction dir)
{
    switch (dir)
    {
        case NORTH: return SOUTH;
        case SOUTH: return NORTH;
        case EAST:  return WEST;
        case WEST:  return EAST;
        case UP:    return DOWN;
        case DOWN:  return UP;
    }
}

Location location_empty(void)
{
    return location_create(0, 0, 0);
}

Location location_from_values(int values[])
{
    return location_create(values[0], values[1], values[2]);
}

Location location_create(Coord x, Coord y, Coord z)
{
    return (Location){x, y, z};
}

Location location_move(Location loc, Direction dir, int length)
{
    switch (dir)
    {
        case NORTH: return (Location){loc.x, loc.y, loc.z - length};
        case SOUTH: return (Location){loc.x, loc.y, loc.z + length};
        case EAST:  return (Location){loc.x + length, loc.y, loc.z};
        case WEST:  return (Location){loc.x - length, loc.y, loc.z};
        case UP:    return (Location){loc.x, loc.y + length, loc.z};
        case DOWN:  return (Location){loc.x, loc.y - length, loc.z};
    }
}

int location_equals(Location l1, Location l2)
{
    return l1.x == l2.x && l1.y == l2.y && l1.z == l2.z;
}

int location_hash(Location loc, int max)
{
    return abs((loc.x * MAGIC_HASH_NUMBER + loc.y) * MAGIC_HASH_NUMBER + loc.z) % max;
}

Block block_empty(void)
{
    return block_create(location_empty(), EMPTY, NORTH);
}

Block block_from_values(int values[])
{
    return block_create(location_from_values(values), values[3], values[4]);
}

Block block_create(Location location, Material material, Direction direction)
{
    return (Block){material, location, direction, 0, 0};
}

void block_allocate(Block** block, Location location, Material material, Direction direction)
{
    *block = malloc(sizeof(Block));
    **block = block_create(location, material, direction);
}

void block_copy(Block* dest, Block* source)
{
    memcpy(dest, source, sizeof(Block));
}

void block_print(Block* block)
{
    if (material_has_direction(block->material))
    {
        printf("(%d,%d,%d) %d %s %s\n",
               block->location.x,
               block->location.y,
               block->location.z,
               block->power,
               Materials[block->material],
               Directions[block->direction]);
    }
    else
    {
        printf("(%d,%d,%d) %d %s\n",
               block->location.x,
               block->location.y,
               block->location.z,
               block->power,
               Materials[block->material]);
    }
}
