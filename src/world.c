/* world.c - Data structure for storing and manipulating nodes
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

static void world_node_move(World* world, Node* node, Direction direction)
{
    Type* type = node->data->type;
    Location new_location = location_move(node->location, direction, 1);

    world_remove_node(world, node->location);
    world_set_node(world, new_location, type, NULL);
}

World* world_allocate(unsigned int size, TypeData* type_data)
{
    World* world = malloc(sizeof(World));
    CHECK_OOM(world);

    world->root = node_data_allocate(type_data_get_default_type(type_data));
    world->tree = node_tree_allocate(NULL, 1, world->root);
    hashmap_init(&world->nodes, size);
    hashmap_init(&world->dead, size);
    world->total_nodes = 0;
    world->type_data = type_data;

    // Stats
    world->ticks = 0;
    world->max_inputs = 0;
    world->max_outputs = 0;
    world->max_queued = 0;

    return world;
}

void world_free(World* world)
{
    node_data_free(world->root);
    node_tree_free(world->tree);
    hashmap_free(&world->nodes, (void (*)(void*))node_data_free);
    hashmap_free(&world->dead, (void (*)(void*))node_data_free);
    type_data_free(world->type_data);
    free(world);
}

void world_set_node(World* world, Location location, Type* type, Node* node)
{
    world->tree = node_tree_ensure_depth(world->tree, location);

    Node found;
    node_tree_get(world->tree, location, &found, true);
    assert(!NODE_IS_EMPTY(&found));

    if (found.data->type == NULL)
    {
        Bucket* bucket = hashmap_get(&world->nodes, found.location, true);
        bucket->value = found.data;
        world->total_nodes++;
    }

    found.data->type = type;

    if (node != NULL)
        *node = found;
}

void world_get_node(World* world, Location location, Node* node)
{
    node_tree_get(world->tree, location, node, false);
}

void world_remove_node(World* world, Location location)
{
    Node node;
    node_tree_remove(world->tree, location, &node);
    if (node.data != NULL)
        world->total_nodes--;
    hashmap_remove(&world->nodes, node.location);
    Bucket* bucket = hashmap_get(&world->dead, node.location, true);
    bucket->value = node.data;
}

void world_gc_nodes(World* world)
{
    unsigned int size = world->dead.size;
    hashmap_free(&world->dead, (void (*)(void*))node_data_free);
    hashmap_init(&world->dead, size);
}

void world_get_adjacent_node(World* world, Node* current_node, Direction dir, Node* node)
{
    Location location = location_move(current_node->location, dir, 1);
    node_tree_get(world->tree, location, node, false);
    assert(!NODE_IS_EMPTY(node));
}

#define FOR_REGION(R)\
    int x_start = (R)->x.start > (R)->x.end ? (R)->x.end : (R)->x.start;\
    int x_end   = (R)->x.start > (R)->x.end ? (R)->x.start : (R)->x.end;\
    int y_start = (R)->y.start > (R)->y.end ? (R)->y.end : (R)->y.start;\
    int y_end   = (R)->y.start > (R)->y.end ? (R)->y.start : (R)->y.end;\
    int z_start = (R)->z.start > (R)->z.end ? (R)->z.end : (R)->z.start;\
    int z_end   = (R)->z.start > (R)->z.end ? (R)->z.start : (R)->z.end;\
    int x_step  = abs((R)->x.step);\
    int y_step  = abs((R)->y.step);\
    int z_step  = abs((R)->z.step);\
    for (int x = x_start; x <= x_end; x += x_step)\
    for (int y = y_start; y <= y_end; y += y_step)\
    for (int z = z_start; z <= z_end; z += z_step)

void world_get_region(World* world, Region* region, void (*callback)(Location l, Node* n, void* args), void* args)
{
    FOR_REGION(region)
    {
        Location location = location_create(x, y, z);
        Node node;
        world_get_node(world, location, &node);
        callback(location, &node, args);
    }
}

void world_set_region(World* world, Region* region, void (*callback)(Location l, Node* n, void* args), void* args)
{
    Location max_range = location_create(
            MAX(abs(region->x.start), abs(region->x.end)),
            MAX(abs(region->y.start), abs(region->y.end)),
            MAX(abs(region->z.start), abs(region->z.end)));
    world->tree = node_tree_ensure_depth(world->tree, max_range);

    FOR_REGION(region)
    {
        Location location = location_create(x, y, z);
        Node node;
        node_tree_get(world->tree, location, &node, true);
        assert(!NODE_IS_EMPTY(&node));
        Type* oldType = node.data->type;

        callback(location, &node, args);

        if (oldType == NULL && node.data->type != NULL)
        {
            Bucket* bucket = hashmap_get(&world->nodes, node.location, true);
            bucket->value = node.data;
            world->total_nodes++;
        }
    }
}

void world_delete_region(World* world, Region* region)
{
    FOR_REGION(region)
    {
        Location location = location_create(x, y, z);
        world_remove_node(world, location);
    }
}

WorldStats world_get_stats(World* world)
{
    return (WorldStats){
        world->ticks,
        world->total_nodes,
        world->tree->level,
        world->nodes.size,
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
    STAT_PRINT(stats, tree_depth, u);
    STAT_PRINT(stats, hashmap_size, u);
    STAT_PRINT(stats, message_max_inputs, u);
    STAT_PRINT(stats, message_max_outputs, u);
    STAT_PRINT(stats, message_max_queued, u);
}

bool world_run_data(World* world, QueueData* data)
{
    switch (data->type)
    {
        case SM_FIELD: {
            unsigned int field_index = data->index;
            switch (data->source.data->type->fields->data[field_index].type)
            {
                case FIELD_INTEGER:
                    if (data->value.integer == FIELD_GET(&data->source, field_index, integer))
                        return false;
                    FIELD_SET(&data->source, field_index, integer, data->value.integer);
                    break;

                case FIELD_DIRECTION:
                    if (data->value.direction == FIELD_GET(&data->source, field_index, direction))
                        return false;
                    FIELD_SET(&data->source, field_index, direction, data->value.direction);
                    break;

                case FIELD_STRING:
                    if (data->value.string == FIELD_GET(&data->source, field_index, string))
                        return false;
                    FIELD_SET(&data->source, field_index, string, data->value.string);
                    break;
            }
        } break;

        case SM_MOVE:
            if (!hashmap_get(&world->dead, data->source.location, false))
                world_node_move(world, &data->source, data->value.direction);
            break;

        case SM_REMOVE:
            if (!hashmap_get(&world->dead, data->source.location, false))
                world_remove_node(world, data->source.location);
            break;

        case SM_DATA:
            /* noop */
            break;
            
        default:
            ERROR("Unknown system message");
    }

    return true;
}

void world_print_messages(World* world)
{
    Node node;
    Cursor cursor = hashmap_get_iterator(&world->nodes);
    while (cursor_next(&cursor, &node.location, (void**)&node.data))
    {
        MessageStore* store = node.data->store;
        while (store != NULL)
        {
            if (store->tick < world->ticks)
                continue;

            for (unsigned int j = 0; j < store->messages->size; j++)
            {
                Message* inst = store->messages->data + j;
                repl_print("%llu %d,%d,%d => %d,%d,%d ",
                        store->tick - world->ticks,
                        inst->source.location.x,
                        inst->source.location.y,
                        inst->source.location.z,
                        node.location.x,
                        node.location.y,
                        node.location.z);

                for (MessageType* mt = world->type_data->message_types; mt != NULL; mt = mt->next)
                {
                    if (mt->id == inst->type)
                    {
                        // TODO: Print additional types
                        repl_print("%s %d\n", mt->name, inst->value);
                        break;
                    }
                }
            }

            store = store->next;
        }
    }
}

void world_print_types(World* world)
{
    FOR_TYPES(type, world->type_data)
    {
        repl_print("%s\n", type->name);
    }
}

void world_print_type(World* world, const char* name)
{
    Type* type = type_data_find_type(world->type_data, name);
    repl_print("Name: %s\n", name);
    repl_print("Fields:\n");
    for (unsigned int i = 0; i < type->fields->count; i++)
    {
        Field* field = type->fields->data + i;
        char* type;
        switch (field->type)
        {
            case FIELD_INTEGER: type = "INTEGER"; break;
            case FIELD_DIRECTION: type = "DIRECTION"; break;
            case FIELD_STRING: type = "STRING"; break;
            default: ERROR("Unknown type");
        }
        repl_print("  %d: %s %s\n", i, field->name, type);
    }

    repl_print("Behaviors:\n");
    for (unsigned int i = 0; i < type->behaviors->count; i++)
    {
        Behavior* behavior = type->behaviors->data[i];
        repl_print("  %d: %s\n", i, behavior->name);
    }
}

