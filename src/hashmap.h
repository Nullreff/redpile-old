/* hashmap.h - Location based hashmap
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

#ifndef REDPILE_BUCKET_H
#define REDPILE_BUCKET_H

#include "redpile.h"
#include "location.h"
#include <stdbool.h>

typedef struct Bucket {
    Location key;
    void* value;
    struct Bucket* next;
} Bucket;

typedef struct {
    Bucket* data;

    // Stats
    unsigned int size;
    unsigned int min_size;
    unsigned int overflow;
    unsigned int resizes;
    unsigned int max_depth;
} Hashmap;

// From http://stackoverflow.com/a/365068
#define ROUND_TO_POW_2(x)\
    x--;\
    x |= x >> 1;\
    x |= x >> 2;\
    x |= x >> 4;\
    x |= x >> 8;\
    x |= x >> 16;\
    x++

Hashmap* hashmap_allocate(unsigned int size);
void hashmap_free(Hashmap* map);
Bucket* hashmap_get(Hashmap* hashmap, Location key, bool create);
void* hashmap_remove(Hashmap* hashmap, Location key);

#endif
