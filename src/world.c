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
#include "block.h"
#include "hashmap.h"

static BlockNode* world_get_node(World* world, Location location)
{
    Bucket* bucket = hashmap_get(world->hashmap, location, false);
    return bucket != NULL ? bucket->value : NULL;
}

static void world_update_adjacent_nodes(World* world, BlockNode* node)
{
    for (int i = 0; i < 6; i++)
    {
        if (node->adjacent[i] == NULL)
        {
            // Find the bucket next to us
            Direction dir = (Direction)i;
            Location location = location_move(node->location, dir, 1);
            BlockNode* found_node = world_get_node(world, location);

            if (found_node != NULL)
            {
                // Update it to point both ways
                found_node->adjacent[direction_invert(dir)] = node;
                node->adjacent[i] = found_node;
            }
        }
    }
}

static void world_reset_adjacent_nodes(World* world, BlockNode* node)
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

static void world_instructions_free(World* world)
{
    for (int i = 0; i < world->instructions->size; i++)
    {
        for (Bucket* bucket = world->instructions->data + i; bucket != NULL; bucket = bucket->next)
        {
            RupQueue* queue = bucket->value;
            while (queue != NULL)
            {
                RupQueue* temp = queue->next;
                rup_queue_free(queue);
                queue = temp;
            }
        }
    }

    hashmap_free(world->instructions);
}

static bool block_missing_noop(Location location, Block* block)
{
    return false;
}

static bool world_fill_missing(World* world, Location location)
{
    Block block = block_empty();
    block.system = true;
    if (world->block_missing(location, &block))
    {
        world_set_block(world, location, &block);
        return true;
    }
    return false;
}

static void world_remove_block(World* world, Location location)
{
    BlockNode* node = hashmap_remove(world->hashmap, location);
    if (node != NULL)
    {
        world_reset_adjacent_nodes(world, node);
        block_list_remove(world->blocks, node);
    }
}

static void world_block_move(World* world, BlockNode* node, Direction direction)
{
    Block copy = node->block;
    Location new_location = location_move(node->location, direction, 1);

    world_remove_block(world, node->location);
    world_set_block(world, new_location, &copy);
}

World* world_allocate(unsigned int size)
{
    World* world = malloc(sizeof(World));
    CHECK_OOM(world);

    world->ticks = 0;
    world->hashmap = hashmap_allocate(size);
    world->blocks = block_list_allocate();
    world->block_missing = block_missing_noop;
    world->instructions = hashmap_allocate(size);

    return world;
}

void world_free(World* world)
{
    hashmap_free(world->hashmap);
    block_list_free(world->blocks);
    world_instructions_free(world);
    free(world);
}

void world_set_block(World* world, Location location, Block* block)
{
    if (block->material == EMPTY)
    {
        world_remove_block(world, location);
        return;
    }

    Bucket* bucket = hashmap_get(world->hashmap, location, true);

    if (bucket->value != NULL)
        block_list_remove(world->blocks, bucket->value);

    bucket->value = block_list_append(world->blocks, location, block);
    world_update_adjacent_nodes(world, bucket->value);
}

Block* world_get_block(World* world, Location location)
{
    BlockNode* node = world_get_node(world, location);
    return node != NULL ? &node->block : NULL;
}

BlockNode* world_get_adjacent_block(World* world, BlockNode* node, Direction dir)
{
    BlockNode* adjacent = node->adjacent[dir];
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
        world->blocks->size,
        world->hashmap->size,
        world->hashmap->overflow,
        world->hashmap->resizes,
        world->hashmap->max_depth
    };
}

void world_stats_print(WorldStats stats)
{
    STAT_PRINT(stats, ticks, llu);
    STAT_PRINT(stats, blocks, u);
    STAT_PRINT(stats, hashmap_allocated, u);
    STAT_PRINT(stats, hashmap_overflow, u);
    STAT_PRINT(stats, hashmap_resizes, u);
    STAT_PRINT(stats, hashmap_max_depth, u);
}

void world_set_block_missing_callback(World* world, bool (*callback)(Location location, Block* node))
{
    world->block_missing = callback;
}

void world_clear_block_missing_callback(World* world)
{
    world->block_missing = block_missing_noop;
}

bool world_run_rup(World* world, RupNode* rup_node)
{
    Location target_loc = rup_node->target->location;
    switch (rup_node->inst.command)
    {
        case RUP_HALT:
            // NOOP
            break;

        case RUP_POWER:
            if (rup_node->inst.source->block.power == rup_node->inst.value.power)
                return false;
            rup_node->inst.source->block.power = rup_node->inst.value.power;
            break;

        case RUP_MOVE:
            world_block_move(world, rup_node->target, rup_node->inst.value.direction);
            world_fill_missing(world, target_loc);
            break;

        case RUP_REMOVE:
            world_remove_block(world, rup_node->target->location);
            break;
    }

    return true;
}

RupInst* world_find_instructions(World* world, BlockNode* node)
{
    Bucket* bucket = hashmap_get(world->instructions, node->location, false);
    if (bucket == NULL)
        return NULL;

    bucket->value = rup_queue_discard_old(bucket->value, world->ticks);
    RupQueue* queue = (RupQueue*)bucket->value;
    if (queue == NULL)
        return NULL;

    RupInst* insts = rup_queue_find_instructions(queue, world->ticks);
    if (insts == NULL)
        return NULL;

    return insts;
}
