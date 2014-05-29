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

#include "location.h"
#include "redpile.h"

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
    for (int i = 0; i < DIRECTIONS_COUNT; i++)
    {
        if (strcasecmp(direction, Directions[i]) == 0)
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
        default:    ERROR("Invalid direction provided to direction_invert\n");
    }
}

Direction direction_right(Direction dir)
{
    assert(dir != UP);
    assert(dir != DOWN);

    switch (dir)
    {
        case NORTH: return EAST;
        case SOUTH: return WEST;
        case EAST:  return SOUTH;
        case WEST:  return NORTH;
        default:    ERROR("Invalid direction provided to direction_right\n");
    }
}

Direction direction_left(Direction dir)
{
    assert(dir != UP);
    assert(dir != DOWN);

    switch (dir)
    {
        case NORTH: return WEST;
        case SOUTH: return EAST;
        case EAST:  return NORTH;
        case WEST:  return SOUTH;
        default:    ERROR("Invalid direction provided to direction_left\n");
    }
}

Location location_empty(void)
{
    return location_create(0, 0, 0);
}

Location location_max(void)
{
    return (Location){INT_MAX, INT_MAX, INT_MAX};
}

Location location_from_values(int values[])
{
    return location_create(values[0], values[1], values[2]);
}

Location location_random(void)
{
    return location_create(rand(), rand(), rand());
}

Location location_create(Coord x, Coord y, Coord z)
{
    // We use INT_MAX for other purposes
    assert(x != INT_MAX);
    assert(y != INT_MAX);
    assert(z != INT_MAX);

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
        default:    ERROR("Invalid direction provided to location_move\n");
    }
}

bool location_equals(Location l1, Location l2)
{
    return l1.x == l2.x && l1.y == l2.y && l1.z == l2.z;
}

bool location_is_maxed(Location loc)
{
    return loc.x == INT_MAX ||
           loc.y == INT_MAX ||
           loc.z == INT_MAX;
}

unsigned int location_hash_unbounded(Location loc)
{
    unsigned int total = 0;
    total += (unsigned int)loc.x;
    total *= MAGIC_HASH_NUMBER;
    total += (unsigned int)loc.y;
    total *= MAGIC_HASH_NUMBER;
    total += (unsigned int)loc.z;
    return total;
}

unsigned int location_hash(Location loc, unsigned int max)
{
    // Our hashing function requires a power of two size
    assert(IS_POWER_OF_2(max));

    unsigned int total = 0;
    total += (unsigned int)loc.x;
    total *= MAGIC_HASH_NUMBER;
    total += (unsigned int)loc.y;
    total *= MAGIC_HASH_NUMBER;
    total += (unsigned int)loc.z;

    // Same as (total % max) when max is a power of two
    return total & (max - 1);
}

