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

#include "node.h"
#include "hashmap.h"
#include "redpile.h"
#include "rup.h"

typedef struct {
    // All nodes are stored in a linked list.
    // See block.c for more information.
    NodeList* nodes;

    // Fast block lookup is done using a hashmap.
    // See hashmap.c for more information.
    Hashmap* hashmap;

    // Used for calculating and storing missing
    // blocks during a redstone tick.
    bool (*node_missing)(Location location, Type* type);

    Hashmap* instructions;

    // Additional stats
    unsigned long long ticks; // Redstone ticks
} World;

typedef struct {
    unsigned long long ticks;
    unsigned int nodes;
    unsigned int hashmap_allocated;
    unsigned int hashmap_overflow;
    unsigned int hashmap_resizes;
    unsigned int hashmap_max_depth;
} WorldStats;

#define STAT_PRINT(stats,stat,format) printf(#stat ": %" #format "\n", stats.stat)

World* world_allocate(unsigned int size);
void world_free(World* world);
Node* world_set_node(World* world, Location location, Type type, bool system);
Node* world_get_node(World* world, Location location);
void world_remove_node(World* world, Location location);
Node* world_get_adjacent_node(World* world, Node* node, Direction dir);
WorldStats world_get_stats(World* world);
void world_stats_print(WorldStats world);
void world_set_node_missing_callback(World* world, bool (*callback)(Location location, Type* type));
void world_clear_node_missing_callback(World* world);
bool world_run_rup(World* world, RupNode* rup_node);
RupInst* world_find_instructions(World* world, Node* node);

#endif

