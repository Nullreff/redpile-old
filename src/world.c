/* world.c - Data structure for storing a redstone simulation
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

#include "world.h"
#include "hashmap.h"
#include "repl.h"

static void world_update_adjacent_nodes(World* world, Node* node)
{
    for (int i = 0; i < DIRECTIONS_COUNT; i++)
    {
        if (node->adjacent[i] == NULL)
        {
            // Find the bucket next to us
            Direction dir = (Direction)i;
            Location location = location_move(node->location, dir, 1);
            Node* found_node = world_get_node(world, location);

            if (found_node != NULL)
            {
                // Update it to point both ways
                found_node->adjacent[direction_invert(dir)] = node;
                node->adjacent[i] = found_node;
            }
        }
    }
}

static void world_reset_adjacent_nodes(World* world, Node* node)
{
    for (int i = 0; i < DIRECTIONS_COUNT; i++)
    {
        if (node->adjacent[i] != NULL)
        {
            Direction dir = (Direction)i;
            node->adjacent[i]->adjacent[direction_invert(dir)] = NULL;
        }
    }
}

static Type* node_missing_noop(TypeData* type_data, Location location)
{
    return NULL;
}

static Type* node_missing_default_type(TypeData* type_data, Location location)
{
    return type_data_get_default_type(type_data);
}

static bool world_fill_missing(World* world, Location location)
{
    Type* type = world->node_missing(world->type_data, location);
    if (type != NULL)
    {
        world_set_node(world, location, type);
        return true;
    }
    return false;
}

static void world_node_move(World* world, Node* node, Direction direction)
{
    Type* type = node->type;
    Location new_location = location_move(node->location, direction, 1);

    world_remove_node(world, node->location);
    world_set_node(world, new_location, type);
}

World* world_allocate(unsigned int size, TypeData* type_data)
{
    World* world = malloc(sizeof(World));
    CHECK_OOM(world);

    world->hashmap = hashmap_allocate(size);
    world->nodes = node_list_allocate();
    world->type_data = type_data;
    world->node_missing = node_missing_noop;

    // Stats
    world->ticks = 0;
    world->max_inputs = 0;
    world->max_outputs = 0;
    world->max_queued = 0;

    return world;
}

void world_free(World* world)
{
    hashmap_free(world->hashmap, NULL);
    node_list_free(world->nodes);
    type_data_free(world->type_data);
    free(world);
}

Node* world_set_node(World* world, Location location, Type* type)
{
    Bucket* bucket = hashmap_get(world->hashmap, location, true);

    if (bucket->value != NULL)
        node_list_remove(world->nodes, bucket->value);

    bucket->value = node_list_append(world->nodes, location, type);
    world_update_adjacent_nodes(world, bucket->value);
    return bucket->value;
}

Node* world_get_node(World* world, Location location)
{
    Bucket* bucket = hashmap_get(world->hashmap, location, false);
    return bucket != NULL ? bucket->value : NULL;
}

void world_remove_node(World* world, Location location)
{
    Node* node = hashmap_remove(world->hashmap, location);
    if (node != NULL)
    {
        world_reset_adjacent_nodes(world, node);
        node_list_remove(world->nodes, node);
    }
}

Node* world_get_adjacent_node(World* world, Node* node, Direction dir)
{
    Node* adjacent = node->adjacent[dir];
    if (adjacent == NULL)
    {
        Location loc = location_move(node->location, dir, 1);
        if (world_fill_missing(world, loc))
            adjacent = node->adjacent[dir];
    }
    return adjacent;
}

WorldStats world_get_stats(World* world)
{
    return (WorldStats){
        world->ticks,
        world->nodes->size,
        world->hashmap->size,
        world->hashmap->overflow,
        world->hashmap->resizes,
        world->hashmap->max_depth,
        world->max_inputs,
        world->max_outputs,
        world->max_queued,
    };
}

#define STAT_PRINT(stats,stat,format) repl_print(#stat ": %" #format "\n", stats.stat)
void world_stats_print(WorldStats stats)
{
    STAT_PRINT(stats, ticks, llu);
    STAT_PRINT(stats, nodes, u);
    STAT_PRINT(stats, hashmap_allocated, u);
    STAT_PRINT(stats, hashmap_overflow, u);
    STAT_PRINT(stats, hashmap_resizes, u);
    STAT_PRINT(stats, hashmap_max_depth, u);
    STAT_PRINT(stats, message_max_inputs, u);
    STAT_PRINT(stats, message_max_outputs, u);
    STAT_PRINT(stats, message_max_queued, u);
}

void world_set_node_missing_callback(World* world, bool enable)
{
    if (enable)
        world->node_missing = node_missing_default_type;
    else
        world->node_missing = node_missing_noop;
}

bool world_run_data(World* world, QueueData* data)
{
    Location target_loc;
    switch (data->type)
    {
        case SM_FIELD: {
            unsigned int field_index = data->value >> 32;
            int field_value = (data->value << 32) >> 32;
            if (field_value == FIELD_GET(data->target.node, field_index, integer))
                return false;
            FIELD_SET(data->target.node, field_index, integer, field_value);
            }
            break;

        case SM_MOVE:
            target_loc = data->source.location;
            world_node_move(world, data->target.node, data->value);
            world_fill_missing(world, target_loc);
            break;

        case SM_REMOVE:
            world_remove_node(world, data->source.location);
            break;
            
        default:
            ERROR("Unknown system message");
    }

    return true;
}

void world_print_messages(World* world)
{
    FOR_NODES(node, world->nodes->active)
    {
        MessageStore* store = node->store;
        while (store != NULL)
        {
            if (store->tick >= world->ticks)
            {
                for (int j = 0; j < store->messages->size; j++)
                {
                    Message* inst = store->messages->data + j;
                    QueueData data = (QueueData) {
                        .source.location = inst->source.location,
                        .source.type = inst->source.type,
                        .target.location = node->location,
                        .tick = store->tick,
                        .type = inst->type,
                        .value = inst->value
                    };
                    queue_data_print_message(&data, world->type_data, world->ticks);
                }
            }
            store = store->next;
        }
    }
}

