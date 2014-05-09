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
        if (strcmp(material, Materials[i]) == 0)
        {
            *result = (Material)i;
            return 0;
        }
    }

    return -1;
}

Block block_empty(void)
{
    return block_create(location_empty(), MATERIAL_DEFAULT, DIRECTION_DEFAULT);
}

Block block_from_values(int values[])
{
    return block_create(location_from_values(values), values[3], values[4]);
}

Block block_create(Location location, Material material, Direction direction)
{
    return (Block){
        location, material, direction,
        {EMPTY_INDEX, EMPTY_INDEX, EMPTY_INDEX,
         EMPTY_INDEX, EMPTY_INDEX, EMPTY_INDEX},
        0, 0
    };
}

void block_print(Block* block)
{
    if (HAS_DIRECTION(block->material))
    {
        printf("(%d,%d,%d) %u %s %s\n",
               block->location.x,
               block->location.y,
               block->location.z,
               block->power,
               Materials[block->material],
               Directions[block->direction]);
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

BlockList* block_list_allocate(unsigned int size)
{
    BlockList* blocks = malloc(sizeof(BlockList));
    CHECK_OOM(blocks);

    // General
    blocks->size = size;
    blocks->index = 0;

    // Stats
    blocks->resizes = 0;

    // Data
    blocks->data = malloc(size * sizeof(Block));
    CHECK_OOM(blocks->data);

    for (int i = 0; i < size; i++)
        blocks->data[i] = block_empty();

    return blocks;
}

void block_list_free(BlockList* blocks)
{
    free(blocks->data);
    free(blocks);
}

// Currently support increasing only
void block_list_resize(BlockList* blocks, unsigned int new_size)
{
    assert(new_size > blocks->size);

    Block* temp = realloc(blocks->data, new_size * sizeof(Block));
    CHECK_OOM(temp);

    blocks->data = temp;

    for (int i = blocks->size; i < new_size; i++)
        blocks->data[i] = block_empty();

    blocks->size = new_size;
    blocks->resizes++;
}

// Retreives the index of the next available block in the world
int block_list_next(BlockList* blocks, bool* resized)
{
    if (blocks->index >= blocks->size)
    {
        block_list_resize(blocks, blocks->size * 2);
        *resized = true;
    }

    return blocks->index++;
}

