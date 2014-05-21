/* world.h - Data structure for storing a redstone simulation
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

#include "block.h"
#include "hashmap.h"
#include "redpile.h"

typedef struct {
    // All blocks are stored in a linked list.
    // See block.c for more information.
    BlockList* blocks;

    // Fast block lookup is done using a hashmap.
    // See hashmap.c for more information.
    Hashmap* hashmap;

    // Additional stats
    unsigned int ticks; // Redstone ticks
} World;

typedef struct {
    unsigned int ticks;
    unsigned int blocks;
    unsigned int power_sources;
    unsigned int hashmap_allocated;
    unsigned int hashmap_overflow;
    unsigned int hashmap_resizes;
    unsigned int hashmap_max_depth;
} WorldStats;

#define NODE_ADJACENT(node,dir) node->adjacent[dir]
#define STAT_PRINT(stats,stat) printf(#stat ": %u\n", stats.stat)
#define BLOCK_INDEX(world,block) (block - world->blocks->data)
#define INDEX_BLOCK(world,index) (world->blocks->data + index)

World* world_allocate(unsigned int size);
void world_free(World* world);
void world_set_block(World* world, Block* block);
Block* world_get_block(World* world, Location location);
int world_get_last_power(World* world, Block* block);
void world_set_last_power(World* world, Block* block);
void world_reset_last_power(World* world, int index);
WorldStats world_get_stats(World* world);
void world_stats_print(WorldStats world);

#endif

