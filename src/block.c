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

Location location_move(Location loc, Direction dir, int length)
{
    switch (dir)
    {
        case D_NORTH: return (Location){loc.x, loc.y, loc.z - length};
        case D_SOUTH: return (Location){loc.x, loc.y, loc.z + length};
        case D_EAST: return (Location){loc.x + length, loc.y, loc.z};
        case D_WEST: return (Location){loc.x - length, loc.y, loc.z};
        case D_UP: return (Location){loc.x, loc.y + length, loc.z};
        case D_DOWN: return (Location){loc.x, loc.y - length, loc.z};
    }
}

int location_equals(Location l1, Location l2)
{
    return l1.x == l2.x && l1.y == l2.y && l1.z == l2.z;
}

// I chose 101 because it kind of looks like two redstone torches :)
#define MAGIC_HASH_NUBER 101
int location_hash(Location loc, int max)
{
    // If you know of a better hashing method, feel free to alter this one
    return abs((loc.x * MAGIC_HASH_NUBER + loc.y) * MAGIC_HASH_NUBER + loc.z) % max;
}

Block block_empty(void)
{
    return block_create(M_EMPTY, (Location){0, 0, 0});
}

Block block_create(Material material, Location location)
{
    return (Block){material, location, 0, 0};
}

void block_allocate(Block** block, Material material, Location location)
{
    *block = malloc(sizeof(Block));
    **block = block_create(material, location);
}

