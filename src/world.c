/* world.c - Data structure for storing a redstone simulation
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

#include "world.h"
#include "hashmap.h"

static void world_update_adjacent_nodes(World* world, Node* node)
{
    for (int i = 0; i < 6; i++)
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
    for (int i = 0; i < 6; i++)
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
        case MESSAGE_POWER:
            if (FIELD_GET(data->target.node, 0) == data->message)
                return false;
            FIELD_SET(data->target.node, 0, data->message);
            break;

        case MESSAGE_PUSH:
        case MESSAGE_PULL:
            target_loc = data->source.location;
            world_node_move(world, data->target.node, data->message);
            world_fill_missing(world, target_loc);
            break;

        case MESSAGE_REMOVE:
            world_remove_node(world, data->source.location);
            break;
    }

    return true;
}

void world_print_messages(World* world)
{
    FOR_NODE_LIST(node, world->nodes)
    {
        MessageStore* store = node->store;
        while (store != NULL);
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
                        .message = inst->value
                    };
                    queue_data_print_verbose(&data, message_type_print, world->ticks);
                }
            }
            store = store->next;
        }
    }
}

