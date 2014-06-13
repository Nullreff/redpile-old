/* node.c - Node storage
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

#include "node.h"
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
    "COMPARATOR",
    "SWITCH"
};

static Node node_create(Location location, Block block)
{
    return (Node){location, block, {NULL, NULL, NULL, NULL, NULL, NULL}, NULL, NULL, false};
}

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
    return block_create(MATERIAL_DEFAULT, DIRECTION_DEFAULT, 0);
}

Block block_from_values(int values[])
{
    return block_create(values[0], values[1], values[2]);
}

Block block_random(void)
{
    Material mat = (Material)(rand() % MATERIALS_COUNT);
    Direction dir = (Direction)(rand() % 4);
    unsigned int state = rand() % 2;
    return block_create(mat, dir, state);
}

Block block_create(Material material, Direction direction, unsigned int state)
{
    return (Block){
        // General information
        material,
        direction,
        state,

        // Redstone state
        0,     // power
    };
}

void block_print(Block* block)
{
    printf("%s %s %u %u\n",
           Materials[block->material],
           Directions[block->direction],
           block->power,
           block->state);
}

void node_print_power(Node* node)
{
    printf("(%d,%d,%d) %u\n",
           node->location.x,
           node->location.y,
           node->location.z,
           node->block.power);
}

NodeList* node_list_allocate(void)
{
    NodeList* blocks = calloc(1, sizeof(NodeList));
    CHECK_OOM(blocks);
    return blocks;
}

void node_list_free(NodeList* blocks)
{
    Node* node = blocks->nodes;
    while (node != NULL)
    {
        Node* temp = node->next;
        free(node);
        node = temp;
    }
    free(blocks);
}

Node* node_list_append(NodeList* blocks, Location location, Block* block, bool system)
{
    Node* node = malloc(sizeof(Node));
    CHECK_OOM(node);

    *node = node_create(location, *block);
    node->system = system;

    if (blocks->nodes != NULL)
    {
        node->next = blocks->nodes;
        blocks->nodes->prev = node;
    }
    blocks->nodes = node;

    blocks->size++;
    return node;
}

void node_list_remove(NodeList* blocks, Node* node)
{
    if (node->prev != NULL)
        node->prev->next = node->next;
    else
        blocks->nodes = node->next;

    if (node->next != NULL)
        node->next->prev = node->prev;

    blocks->size--;
    free(node);
}

void node_list_move_after(NodeList* blocks, Node* node, Node* target)
{
    // Already in the right place
    if (node->next == target)
        return;

    // Remove 'target' from the list
    if (target->prev != NULL)
        target->prev->next = target->next;
    else
        blocks->nodes = target->next;

    if (target->next != NULL)
        target->next->prev = target->prev;

    // Add 'target' after 'node'
    if (node->next != NULL)
    {
        target->next = node->next;
        node->next->prev = target;
    }

    node->next = target;
    target->prev = node;
}

void node_list_print(NodeList* blocks)
{
    FOR_BLOCK_LIST(blocks)
    {
        block_print(&node->block);
    }

    printf("Total: %u\n", blocks->size);
}

