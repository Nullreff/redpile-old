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
    return BUCKET_FILLED(bucket) ? bucket->value : NULL;
}

static void world_update_adjacent_nodes(World* world, BlockNode* node)
{
    for (int i = 0; i < 6; i++)
    {
        if (node->adjacent[i] == NULL)
        {
            // Find the bucket next to us
            Direction dir = (Direction)i;
            Location location = location_move(node->block.location, dir, 1);
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

World* world_allocate(unsigned int size)
{
    World* world = malloc(sizeof(World));
    CHECK_OOM(world);

    world->ticks = 0;

    world->hashmap = hashmap_allocate(size);
    world->blocks = block_list_allocate();

    return world;
}

void world_free(World* world)
{
    hashmap_free(world->hashmap);
    block_list_free(world->blocks);
    free(world);
}

void world_set_block(World* world, Block* block)
{
    if (block->material == EMPTY)
    {
        BlockNode* node = hashmap_remove(world->hashmap, block->location);
        if (node != NULL)
        {
            world_reset_adjacent_nodes(world, node);
            block_list_remove(world->blocks, node);
        }
        return;
    }

    Bucket* bucket = hashmap_get(world->hashmap, block->location, true);

    // Attach the next availabe block to this bucket
    if (bucket->value == NULL)
    {
        bucket->value = block_list_append(world->blocks, block);
        world_update_adjacent_nodes(world, bucket->value);
    }
    else
    {
        BlockNode* node = bucket->value;
        memcpy(&node->block, block, sizeof(Block));
    }
}

Block* world_get_block(World* world, Location location)
{
    BlockNode* node = world_get_node(world, location);
    return node != NULL ? &node->block : NULL;
}

BlockNode* world_get_adjacent_block(World* world, BlockNode* node, Direction dir)
{
    return node->adjacent[dir];
}

WorldStats world_get_stats(World* world)
{
    return (WorldStats){
        world->ticks,
        world->blocks->size,
        world->blocks->power_sources,
        world->hashmap->size,
        world->hashmap->overflow,
        world->hashmap->resizes,
        world->hashmap->max_depth
    };
}

void world_stats_print(WorldStats stats)
{
    STAT_PRINT(stats, ticks);
    STAT_PRINT(stats, blocks);
    STAT_PRINT(stats, power_sources);
    STAT_PRINT(stats, hashmap_allocated);
    STAT_PRINT(stats, hashmap_overflow);
    STAT_PRINT(stats, hashmap_resizes);
    STAT_PRINT(stats, hashmap_max_depth);
}

