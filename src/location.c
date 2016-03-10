/* location.c - Tools for tracking x,y,z coordinates
 *
 * Copyright (C) 2014 Ryan Mendivil <ryan@nullreff.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redpile nor the names of its contributors may be
 *     used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
        default:
            WARN("Invalid direction provided to direction_invert: %d\n", dir);
            return 0;
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
        default:
            WARN("Invalid direction provided to direction_right: %d\n", dir);
            return 0;
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
        default:
            WARN("Invalid direction provided to direction_left: %d\n", dir);
            return 0;
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
        default:
           WARN("Invalid movement provided to direction_move: %d\n", move);
           return 0;
    }
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
        default:
            WARN("Invalid direction provided to location_move\n");
            return loc;
    }
}

char direction_to_letter(Direction dir)
{
    switch (dir)
    {
        case NORTH: return 'N';
        case SOUTH: return 'S';
        case EAST:  return 'E';
        case WEST:  return 'W';
        case UP:    return 'U';
        case DOWN:  return 'D';
        default:
            WARN("Invalid direction provided to direction_to_char\n");
            return 'X';
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

Region* region_allocate(Range x, Range y, Range z)
{
    Region* region = malloc(sizeof(Region));
    region->x = x;
    region->y = y;
    region->z = z;
    return region;
}

void region_randomize(Region* region, int size)
{
    int x = rand() % 10000;
    int y = rand() % 10000;
    int z = rand() % 10000;
    region->x = range_create(x, x + size, 1);
    region->y = range_create(y, y + size, 1);
    region->z = range_create(z, z + size, 1);
}

int region_area(Region* region)
{
    return ((abs(region->x.start - region->x.end) + 1) / region->x.step) *
           ((abs(region->y.start - region->y.end) + 1) / region->y.step) *
           ((abs(region->z.start - region->z.end) + 1) / region->z.step);
}

bool region_is_flat(Region* region)
{
    return region->x.start == region->x.end ||
           region->y.start == region->y.end ||
           region->z.start == region->z.end;
}

