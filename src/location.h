/* location.h - Tools for tracking x,y,z coordinates
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

#define COORD_EMPTY INT_MIN
typedef int Coord;
typedef struct {
    Coord x;
    Coord y;
    Coord z;
} Location;

Direction direction_invert(Direction dir);
Direction direction_right(Direction dir);
Direction direction_left(Direction dir);
Direction direction_move(Direction direction, Movement move);

Location location_empty(void);
Location location_from_values(int values[]);
Location location_random(void);
Location location_create(Coord x, Coord y, Coord z);
Location location_move(Location loc, Direction dir, int length);
bool location_equals(Location l1, Location l2);
unsigned int location_hash_unbounded(Location loc);
unsigned int location_hash(Location loc, unsigned int max);

#endif
