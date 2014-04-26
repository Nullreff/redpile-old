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

#include "location.h"
#include <stdbool.h>

/*
When designing Redpile, fast block storage and lookup was a top priority.
In order to do this, we needed a data structure that allows:

  1. Fast insertion and update of individual blocks
  2. Fast lookup of blocks by location
  3. Instant lookup of any block adjacent to another
  4. Fast iteration over all blocks

The result of these requirements is rather interesting, it's a hashmap[1][2]
that stores references to all nearby buckets[3] and stores all buckets
sequentially in memory[4].  The memory layout looks something like this:

           (Hashmap)                   (Overflow)
  +--------------------------+--------------------------+
  |* *** **** **** ***** ****|**********                |
  +--------------------------+--------------------------+
  ^                          ^          ^               ^
*data                 hashmap_size    index            size

When inserting, a hash is calculated that determines where the bucket is 
placed in the (Hashmap) portion of the data structure.  If there's already
a bucket at that location, it's added to the (Overflow) section at 'index'
and a pointer is added from the bucket that lives in (Hashmap).  After it's
inserted, all 6 adjacent buckets to it are looked up using the same method
and stored in the 'adjacent' array.  If new blocks are inserted or updated,
the same buckets will still be used to identify them so any overhead from
finding adjacent blocks goes away after the first insertion.

Currently the only performance issue with this design is rebuilding the
hashmap when it becomes too large.  However, this is simply a case of
optimizing for redstone tick performance over the insertion of large amounts
of new blocks during runtime and is (in my opinion) an acceptable trade off.
*/

typedef struct Bucket {
    // Blocks are indexed by location.  A copy is stored on this struct so we
    // don't have to look up the corresponding block to check for a match.
    Location key;

    // The index in the block list where the block referenced by this bucket can
    // be found.  We don't use a pointer because resizing the block array with
    // realloc sometimes causes it to be moved around in memory.
    unsigned int index;

    // We keep references to the 6 blocks adjacent to this one for faster
    // access during redstone ticks.  This adds a bit of extra time to
    // insertions but more than makes up for it in situation where you're
    // following a chain of blocks.
    struct Bucket* adjacent[6];

    // In cases where we run into hash collisions, a linked list of buckets is
    // used to store extras.
    struct Bucket* next;
} Bucket;

typedef struct {
    // All buckets are stored here, both top level buckets and those found by
    // following *next pointers.
    Bucket* data;

    // The next available empty bucket in *data.
    unsigned int index;

    // Total number of allocated buckets in *data.
    unsigned int size;

    // The number of buckets at the beginning of *data that are top level
    // buckets in the hashmap.
    unsigned int hashmap_size;

    // Stats
    unsigned int resizes;
} BucketList;

#define BUCKET_FILLED(bucket) (bucket != NULL && bucket->index != -1)
#define BUCKET_ADJACENT(bucket,dir) bucket->adjacent[dir]

void bucket_list_print(BucketList* buckets, Bucket* selected);
BucketList* bucket_list_allocate(unsigned int size);
void bucket_list_free(BucketList* map);
Bucket* bucket_list_get(BucketList* map, Location key, bool create);

#endif
