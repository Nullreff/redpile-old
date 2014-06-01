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
    "INSULATOR",
    "WIRE",
    "CONDUCTOR",
    "TORCH",
    "PISTON",
    "REPEATER",
    "COMPARATOR"
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

Block block_from_location(Location loc)
{
    Material mat = (Material)(location_hash_unbounded(loc) % MATERIALS_COUNT);
    Direction dir = (Direction)location_hash(loc, 4);
    unsigned int state = location_hash(loc, 2);
    return block_create(loc, mat, dir, state);
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
        0,     // power
        0,     // power_state
        false, // powered
        false, // modified
        false  // system
    };
}

static BlockNode block_node_create(Block block)
{
    return (BlockNode){block, {NULL, NULL, NULL, NULL, NULL, NULL}, NULL, NULL};
}

static BlockType block_get_type(Block* block)
{
    if M_BOUNDARY(block->material)
        return BOUNDARY;
    else if M_UNPOWERABLE(block->material)
        return UNPOWERABLE;
    else
        return POWERABLE;
}

void block_print(Block* block)
{
    if (M_HAS_DIRECTION(block->material))
    {
        if (M_HAS_STATE(block->material))
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

void block_list_print(BlockList* blocks)
{
    for (int i = 0; i < BLOCK_TYPE_COUNT; i++)
    {
        BlockNode* node;
        FOR_BLOCK_LIST(node, blocks, i)
            block_print(&node->block);
        printf("Total: %u\n", blocks->sizes[i]);
    }

    printf("Grand Total: %u\n", blocks->total);
}

BlockList* block_list_allocate(void)
{
    BlockList* blocks = calloc(1, sizeof(BlockList));
    CHECK_OOM(blocks);
    return blocks;
}

void block_list_free(BlockList* blocks)
{
    for (int i = 0; i < BLOCK_TYPE_COUNT; i++)
    {
        BlockNode* node = blocks->nodes[i];
        while (node != NULL)
        {
            BlockNode* temp = node->next;
            free(node);
            node = temp;
        }
    }
    free(blocks);
}

BlockNode* block_list_append(BlockList* blocks, Block* block)
{
    BlockNode* node = malloc(sizeof(BlockNode));
    CHECK_OOM(node);

    BlockType type = block_get_type(block);
    *node = block_node_create(*block);

    if (blocks->nodes[type] != NULL)
    {
        node->next = blocks->nodes[type];
        blocks->nodes[type]->prev = node;
    }
    blocks->nodes[type] = node;

    blocks->sizes[type]++;
    blocks->total++;
    return node;
}

void block_list_remove(BlockList* blocks, BlockNode* node)
{
    BlockType type = block_get_type(&node->block);
    if (node->prev != NULL)
        node->prev->next = node->next;
    else
        blocks->nodes[type] = node->next;

    if (node->next != NULL)
        node->next->prev = node->prev;

    blocks->total--;
    blocks->sizes[type]--;

    free(node);
}

