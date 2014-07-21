/* location.c - Tools for tracking x,y,z coordinates
 *
 * Copyright (C) 2014 Ryan Mendivil <ryan@nullreff.net>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.  *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "location.h"
#include "common.h"

char* Directions[6] = {
    "NORTH",
    "SOUTH",
    "EAST",
    "WEST",
    "UP",
    "DOWN"
};

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
        default:    ERROR("Invalid direction provided to direction_invert: %d\n", dir);
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
        default:    ERROR("Invalid direction provided to direction_right: %d\n", dir);
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
        default:    ERROR("Invalid direction provided to direction_left: %d\n", dir);
    }
}

Direction direction_move(Direction direction, Movement move)
{
    switch (move)
    {
        case FORWARDS: return direction;
        case BEHIND:   return direction_invert(direction);
        case LEFT:     return direction_left(direction);
        case RIGHT:    return direction_right(direction);
        default:       ERROR("Invalid movement provided to direction_move: %d\n", move);
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

Location location_random(void)
{
    return location_create(rand(), rand(), rand());
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
        default:    ERROR("Invalid direction provided to location_move\n");
    }
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
    assert(IS_POWER_OF_TWO(max));

    unsigned int total = 0;
    total += (unsigned int)loc.x;
    total *= MAGIC_HASH_NUMBER;
    total += (unsigned int)loc.y;
    total *= MAGIC_HASH_NUMBER;
    total += (unsigned int)loc.z;

    // Same as (total % max) when max is a power of two
    return total & (max - 1);
}

