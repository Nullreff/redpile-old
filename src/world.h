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
#include "common.h"
#include "message.h"
#include "queue.h"

typedef struct {
    // All nodes are stored in a linked list.
    // See node.c for more information.
    NodeList* nodes;

    // Type and behavior information
    // see type.c for more information.
    TypeData* type_data;

    // Fast block lookup is done using a hashmap.
    // See hashmap.c for more information.
    Hashmap* hashmap;

    // Used for calculating and storing missing
    // nodes during a redstone tick.
    Type* (*node_missing)(TypeData* type_data, Location location);

    // Additional stats
    unsigned long long ticks; // Redstone ticks
    unsigned int max_inputs;
    unsigned int max_outputs;
    unsigned int max_queued;
} World;

typedef struct {
    unsigned long long ticks;
    unsigned int nodes;
    unsigned int hashmap_allocated;
    unsigned int hashmap_overflow;
    unsigned int hashmap_resizes;
    unsigned int hashmap_max_depth;
    unsigned int message_max_inputs;
    unsigned int message_max_outputs;
    unsigned int message_max_queued;
} WorldStats;

World* world_allocate(unsigned int size, TypeData* type_data);
void world_free(World* world);
Node* world_set_node(World* world, Location location, Type* type);
Node* world_get_node(World* world, Location location);
void world_remove_node(World* world, Location location);
Node* world_get_adjacent_node(World* world, Node* node, Direction dir);
WorldStats world_get_stats(World* world);
void world_stats_print(WorldStats world);
void world_set_node_missing_callback(World* world, bool enable);
bool world_run_data(World* world, QueueData* data);
void world_print_messages(World* world);

#endif

