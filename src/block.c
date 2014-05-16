/* block.c - Blocks and block related data structures
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

#include "block.h"
#include "redpile.h"

char* Materials[MATERIALS_COUNT] = {
    "EMPTY",
    "AIR",
    "WIRE",
    "CONDUCTOR",
    "INSULATOR",
    "TORCH",
    "REPEATER"
};

int material_parse(char* material, Material* result)
{
    for (int i = 0; i < MATERIALS_COUNT; i++)
    {
        if (strcasecmp(material, Materials[i]) == 0)
        {
            *result = (Material)i;
            return 0;
        }
    }

    return -1;
}

Block block_empty(void)
{
    return block_create(location_empty(), MATERIAL_DEFAULT, DIRECTION_DEFAULT, 0);
}

Block block_from_values(int values[])
{
    return block_create(location_from_values(values), values[3], values[4], values[5]);
}

Block block_create(Location location, Material material, Direction direction, unsigned int state)
{
    return (Block){
        // General information
        location,
        material,
        direction,
        state,

        // Redstone state
        0,    // power
        0,    // power_state
        0,    // last_power
        false // updated
    };
}

static BlockNode block_node_create(Block block)
{
    return (BlockNode){block, {NULL, NULL, NULL, NULL, NULL, NULL}, NULL, NULL};
}

void block_print(Block* block)
{
    if (HAS_DIRECTION(block->material))
    {
        if (HAS_STATE(block->material))
        {
            printf("(%d,%d,%d) %u %s %s %u\n",
                   block->location.x,
                   block->location.y,
                   block->location.z,
                   block->power,
                   Materials[block->material],
                   Directions[block->direction],
                   block->state);
        }
        else
        {
            printf("(%d,%d,%d) %u %s %s\n",
                   block->location.x,
                   block->location.y,
                   block->location.z,
                   block->power,
                   Materials[block->material],
                   Directions[block->direction]);
        }
    }
    else
    {
        printf("(%d,%d,%d) %u %s\n",
               block->location.x,
               block->location.y,
               block->location.z,
               block->power,
               Materials[block->material]);
    }
}

void block_print_power(Block* block)
{
    printf("(%d,%d,%d) %u\n",
           block->location.x,
           block->location.y,
           block->location.z,
           block->power);
}

BlockList* block_list_allocate(void)
{
    BlockList* blocks = malloc(sizeof(BlockList));
    CHECK_OOM(blocks);

    blocks->head = NULL;
    blocks->tail = NULL;
    blocks->size = 0;
    blocks->power_sources = 0;

    return blocks;
}

void block_list_free(BlockList* blocks)
{
    BlockNode* node = blocks->head;
    while (node != NULL)
    {
        BlockNode* temp = node->next;
        free(node);
        node = temp;
    }
    free(blocks);
}

BlockNode* block_list_append(BlockList* blocks, Block* block)
{
    BlockNode* node = malloc(sizeof(BlockNode));
    *node = block_node_create(*block);

    if (blocks->head == NULL)
    {
        blocks->head = node;
        blocks->tail = node;
        if POWER_SOURCE(block->material)
            blocks->power_sources++;
    }
    else
    {
        if POWER_SOURCE(block->material)
        {
            node->next = blocks->head;
            blocks->head->prev = node;
            blocks->head = node;
            blocks->power_sources++;
        }
        else
        {
            node->prev = blocks->tail;
            blocks->tail->next = node;
            blocks->tail = node;
        }
    }

    blocks->size++;
    return node;
}

void block_list_remove(BlockList* blocks, BlockNode* node)
{
    if (node->prev == NULL)
        blocks->head = node->next;
    else
        node->prev->next = node->next;

    if (node->next == NULL)
        blocks->tail = node->prev;
    else
        node->next->prev = node->prev;

    blocks->size--;
    if POWER_SOURCE(node->block.material)
        blocks->power_sources--;
    free(node);
}

