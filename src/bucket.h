/* bucket.h - Location based hashmap with fast access to nearby buckets
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
    // Blocks are indexed by location.  A copy is stored on this struct so we
    // don't have to look up the corresponding block to check for a match.
    Location key;

    // The index in the block list where the block referenced by this bucket can
    // be found.  We don't use a pointer because resizing the block array with
    // realloc sometimes causes it to be moved around in memory.
    unsigned int index;

    // In cases where we run into hash collisions, a linked list of buckets is
    // used to store extras.
    struct Bucket* next;
} Bucket;

typedef struct {
    // All buckets are stored here, both top level buckets and those found by
    // following *next pointers.
    Bucket* data;

    // Total number of allocated buckets in *data.
    unsigned int size;

    // Stats
    unsigned int overflow;
    unsigned int resizes;
    unsigned int max_depth;
} BucketList;

#define BUCKET_FILLED(bucket) (bucket != NULL && bucket->index != EMPTY_INDEX)

void bucket_list_print(BucketList* buckets, Bucket* selected);
BucketList* bucket_list_allocate(unsigned int size);
void bucket_list_free(BucketList* map);
Bucket* bucket_list_get(BucketList* map, Location key, bool create);

#endif
