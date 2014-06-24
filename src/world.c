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

static void rup_queue_free_void(void* value)
{
    rup_queue_free(value);
}

static void world_messages_free(World* world)
{
    hashmap_free(world->messages, rup_queue_free_void);
}

static bool node_missing_noop(Location location, Type* type)
{
    return false;
}

static bool world_fill_missing(World* world, Location location)
{
    Type type;
    if (world->node_missing(location, &type))
    {
        world_set_node(world, location, type);
        return true;
    }
    return false;
}

static void world_node_move(World* world, Node* node, Direction direction)
{
    Type type = node->type;
    Location new_location = location_move(node->location, direction, 1);

    world_remove_node(world, node->location);
    world_set_node(world, new_location, type);
}

World* world_allocate(unsigned int size)
{
    World* world = malloc(sizeof(World));
    CHECK_OOM(world);

    world->ticks = 0;
    world->hashmap = hashmap_allocate(size);
    world->nodes = node_list_allocate();
    world->node_missing = node_missing_noop;
    world->messages = hashmap_allocate(size);

    return world;
}

void world_free(World* world)
{
    hashmap_free(world->hashmap, NULL);
    node_list_free(world->nodes);
    world_messages_free(world);
    free(world);
}

Node* world_set_node(World* world, Location location, Type type)
{
    if (type == EMPTY)
    {
        world_remove_node(world, location);
        return NULL;
    }

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
        world->hashmap->max_depth
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
}

void world_set_node_missing_callback(World* world, bool (*callback)(Location location, Type* type))
{
    world->node_missing = callback;
}

void world_clear_node_missing_callback(World* world)
{
    world->node_missing = node_missing_noop;
}

bool world_run_rup(World* world, RupNode* rup_node)
{
    Location target_loc;
    switch (rup_node->inst.type)
    {
        case RUP_POWER:
            if (FIELD_GET(rup_node->target, 0) == rup_node->inst.message)
                return false;
            FIELD_SET(rup_node->target, 0, rup_node->inst.message);
            break;

        case RUP_MOVE:
            target_loc = rup_node->inst.source.location;
            world_node_move(world, rup_node->target, rup_node->inst.message);
            world_fill_missing(world, target_loc);
            break;

        case RUP_REMOVE:
            world_remove_node(world, rup_node->inst.source.location);
            break;
    }

    return true;
}

RupInsts* world_find_messages(World* world, Node* node)
{
    Bucket* bucket = hashmap_get(world->messages, node->location, false);
    if (bucket == NULL)
        return NULL;

    bucket->value = rup_queue_discard_old(bucket->value, world->ticks);
    RupQueue* queue = (RupQueue*)bucket->value;
    if (queue == NULL)
        return NULL;

    RupInsts* insts = rup_queue_find_instructions(queue, world->ticks);
    if (insts == NULL)
        return NULL;

    return insts;
}

void world_print_messages(World* world)
{
    for (int i = 0; i < world->messages->size; i++)
    {
        Bucket* bucket = world->messages->data + i;
        if (bucket->value == NULL)
            continue;

        do
        {
            RupQueue* queue = bucket->value;
            do
            {
                if (queue->tick >= world->ticks)
                {
                    for (int j = 0; j < queue->insts->size; j++)
                        rup_inst_print_verbose(queue->insts->data + j, queue->tick - world->ticks, bucket->key);
                }
                queue = queue->next;
            }
            while (queue != NULL);
            bucket = bucket->next;
        }
        while (bucket != NULL);
    }
}

