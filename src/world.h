/* world.h - Data structure for storing a redstone simulation
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
    NodeTree* tree;
    NodeList* nodes;
    unsigned int total_nodes;

    // Type and behavior information
    // see type.c for more information.
    TypeData* type_data;

    // Fast block lookup is done using a hashmap.
    // See hashmap.c for more information.

    // If set to true, any non-existant nodes requested
    // will be filled in with the default material.
    bool fill_missing;

    // Additional stats
    unsigned long long ticks; // Redstone ticks
    unsigned int max_inputs;
    unsigned int max_outputs;
    unsigned int max_queued;
} World;

typedef struct {
    unsigned long long ticks;
    unsigned int nodes;
    unsigned int tree_depth;
    unsigned int message_max_inputs;
    unsigned int message_max_outputs;
    unsigned int message_max_queued;
} WorldStats;

World* world_allocate(unsigned int size, TypeData* type_data);
void world_free(World* world);
void world_set_node(World* world, Location location, Type* type, Node* node);
void world_get_node(World* world, Location location, Node* node);
void world_remove_node(World* world, Location location);
void world_get_adjacent_node(World* world, Node* current_node, Direction dir, Node* node);
WorldStats world_get_stats(World* world);
void world_stats_print(WorldStats world);
bool world_run_data(World* world, QueueData* data);
void world_print_messages(World* world);
void world_print_types(World* world);
void world_print_type(World* world, const char* name);

#endif

