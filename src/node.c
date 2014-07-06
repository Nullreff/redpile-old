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

unsigned int FieldCounts[MATERIALS_COUNT] = {0, 0, 0, 1, 1, 2, 2, 3, 3, 3};

static Node* node_allocate(Location location, Type type)
{
    unsigned int count = FieldCounts[type];

    Node* node = calloc(1, sizeof(Node) + (count * sizeof(Field)));
    CHECK_OOM(node);
    node->location = location;
    node->type = type;
    node->store = NULL;
    node->last_input = NULL;
    node->fields.count = count;
    return node;
}

static void node_free(Node* node)
{
    message_store_free(node->store);
    free(node->last_input);
    free(node);
}

Messages* node_find_messages(Node* node, unsigned long long tick)
{
    node->store = message_store_discard_old(node->store, tick);
    MessageStore* store = node->store;
    if (store == NULL)
        return NULL;

    Messages* messages = message_store_find_instructions(store, tick);
    if (messages == NULL)
        return NULL;

    return messages;
}

MessageStore* node_find_store(Node* node, unsigned long long tick)
{
    MessageStore* store = node->store;
    if (store == NULL)
    {
        store = message_store_allocate(tick);
        node->store = store;
    }
    else
    {
        store = message_store_find(store, tick);
        if (store == NULL)
        {
            store = message_store_allocate(tick);
            store->next = node->store;
            node->store = store;
        }
    }

    return store;
}

void node_print(Node* node)
{
    printf("(%d,%d,%d) %s",
           node->location.x,
           node->location.y,
           node->location.z,
           Materials[node->type]);

    // Power
    if (node->fields.count > 0)
        printf(" %u", node->fields.data[0]);

    // Direction
    if (node->fields.count > 1)
        printf(" %s", Directions[node->fields.data[1]]);

    // State
    if (node->fields.count > 2)
        printf(" %u", node->fields.data[2]);

    printf("\n");
}

void node_print_power(Node* node)
{
    printf("(%d,%d,%d) %u\n",
           node->location.x,
           node->location.y,
           node->location.z,
           FIELD_GET(node, 0));
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
        node_free(node);
        node = temp;
    }
    free(blocks);
}

Node* node_list_append(NodeList* blocks, Location location, Type type)
{
    Node* node = node_allocate(location, type);

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
    node_free(node);
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
    target->next = node->next;
    if (node->next != NULL)
        node->next->prev = target;

    node->next = target;
    target->prev = node;
}

void node_list_print(NodeList* blocks)
{
    FOR_NODE_LIST(node, blocks)
    {
        node_print(node);
    }

    printf("Total: %u\n", blocks->size);
}

