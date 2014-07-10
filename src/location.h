/* location.h - Tools for tracking x,y,z coordinates
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

#ifndef REDPILE_LOCATION_H
#define REDPILE_LOCATION_H

#include <stdbool.h>

// I chose 101 because it kind of looks like two redstone torches
// If you have a better prime number, feel free to use it :)
#define MAGIC_HASH_NUMBER 101

#define DIRECTIONS_COUNT 6
char* Directions[DIRECTIONS_COUNT];
typedef enum {
    NORTH = 0,
    SOUTH = 1,
    EAST  = 2,
    WEST  = 3,
    UP    = 4,
    DOWN  = 5
} Direction;

#define MOVEMENTS_COUNT 4
typedef enum {
    FORWARDS = 6,
    BEHIND   = 7,
    LEFT     = 8,
    RIGHT    = 9
} Movement;

typedef int Coord;
typedef struct {
    Coord x;
    Coord y;
    Coord z;
} Location;

#define LOCATION_EQUALS(LOC1,LOC2) (LOC1.x == LOC2.x && LOC1.y == LOC2.y && LOC1.z == LOC2.z)

Direction direction_invert(Direction dir);
Direction direction_right(Direction dir);
Direction direction_left(Direction dir);
Direction direction_move(Direction direction, Movement move);

Location location_empty(void);
Location location_from_values(int values[]);
Location location_random(void);
Location location_create(Coord x, Coord y, Coord z);
Location location_move(Location loc, Direction dir, int length);
unsigned int location_hash_unbounded(Location loc);
unsigned int location_hash(Location loc, unsigned int max);

#endif
