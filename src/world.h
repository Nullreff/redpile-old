/* world.h - Data structure for storing and manipulating blocks
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

#ifndef REDPILE_WORLD_H
#define REDPILE_WORLD_H

#include <stdbool.h>
#include "block.h"
#include "bucket.h"

#define BLOCK_FROM_BUCKET(world,bucket) ((world)->blocks + (bucket)->index)

typedef struct {
    // Block storage/lookup using a basic hashmap
    // Blocks are placed sequentially in `blocks`
    // with a bucket added to `buckets` for lookup
    int buckets_size;
    int blocks_size;
    Bucket* buckets;
    Block* blocks;

    // Stats used by `world_print_status`
    int count;         // Total blocks
    int ticks;         // Redstone ticks
    int max_depth;     // Deepest bucket
    int collisions;    // Number of hash collisions
    int power_sources; // Number of blocks that are power sources
} World;

void world_allocate(World** world_ptr, unsigned int size);
void world_free(World** world_ptr);
Bucket* world_get_bucket(World* world, Location location, bool allocate);
Block* world_set_block(World* world, Block* block);
Block* world_get_block(World* world, Location location);
void world_print_status(World* world);

#endif

