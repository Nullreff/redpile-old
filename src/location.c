/* location.c - Tools for tracking x,y,z coordinates
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
#include "location.h"

char* Directions[6] = {
    "NORTH",
    "SOUTH",
    "EAST",
    "WEST",
    "UP",
    "DOWN"
};

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

