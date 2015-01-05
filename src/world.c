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

    world->tree = node_tree_allocate(0, NULL);
    world->nodes = node_list_allocate(size);
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
    node_tree_free(world->tree);
    node_list_free(world->nodes);
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
        node_list_prepend(&world->nodes, &found);
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
    world_get_node(world, location, &node);
    if (!NODE_IS_EMPTY(&node))
    {
        node_data_free(node.data);
        memset(node.data, 0, sizeof(NodeData));
        world->total_nodes--;
    }
}

void world_get_adjacent_node(World* world, Node* current_node, Direction dir, Node* node)
{
    Location location = location_move(current_node->location, dir, 1);
    world->tree = node_tree_ensure_depth(world->tree, location);

    node_tree_get(world->tree, location, node, true);
    assert(!NODE_IS_EMPTY(node));

    if (node->data->type == NULL)
    {
        node->data->type = type_data_get_default_type(world->type_data);
        node_list_prepend(&world->nodes, node);
        world->total_nodes++;
    }
}

WorldStats world_get_stats(World* world)
{
    return (WorldStats){
        world->ticks,
        world->total_nodes,
        world->tree->level,
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
            switch (data->target.data->type->fields->data[field_index].type)
            {
                case FIELD_INTEGER:
                    if (data->value.integer == FIELD_GET(&data->target, field_index, integer))
                        return false;
                    FIELD_SET(&data->target, field_index, integer, data->value.integer);
                    break;

                case FIELD_DIRECTION:
                    if (data->value.direction == FIELD_GET(&data->target, field_index, direction))
                        return false;
                    FIELD_SET(&data->target, field_index, direction, data->value.direction);
                    break;

                case FIELD_STRING:
                    if (data->value.string == FIELD_GET(&data->target, field_index, string))
                        return false;
                    FIELD_SET(&data->target, field_index, string, data->value.string);
                    break;
            }
        } break;

        case SM_MOVE:
            world_node_move(world, &data->target, data->value.direction);
            break;

        case SM_REMOVE:
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
    FOR_NODES(node, world->nodes)
        MessageStore* store = node->data->store;
        while (store != NULL)
        {
            if (store->tick >= world->ticks)
            {
                for (unsigned int j = 0; j < store->messages->size; j++)
                {
                    Message* inst = store->messages->data + j;
                    QueueData data = (QueueData) {
                        .source = {inst->source.location, inst->source.type},
                        .target = *node,
                        .tick = store->tick,
                        .type = inst->type,
                        .index = 0,
                        .value = { .integer = inst->value }
                    };
                    queue_data_print_message(&data, world->type_data, world->ticks);
                }
            }
            store = store->next;
        }
    FOR_NODES_END
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

