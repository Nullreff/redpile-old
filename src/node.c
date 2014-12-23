/* node.c - Node storage
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

#include "node.h"
#include "repl.h"
#include <limits.h>

Node node_empty(void)
{
    return (Node){location_empty(), NULL};
}

void node_initialize_fields(Node* node)
{
    if (node->data->fields != NULL)
        return;

    unsigned int field_count = node->data->type->fields->count;
    FieldData* fields = calloc(1, sizeof(FieldData) + (sizeof(FieldValue) * field_count));
    fields->count = field_count;
    node->data->fields = fields;
}

Messages* node_find_messages(Node* node, unsigned long long tick)
{
    NodeData* data = node->data;
    MessageStore* store = data->store = message_store_discard_old(data->store, tick);
    if (store == NULL)
        return NULL;

    Messages* messages = message_store_find_instructions(store, tick);
    if (messages == NULL)
        return NULL;

    return messages;
}

MessageStore* node_find_store(Node* node, unsigned long long tick)
{
    NodeData* data = node->data;
    MessageStore* store = data->store;
    if (store == NULL)
    {
        store = message_store_allocate(tick);
        data->store = store;
    }
    else
    {
        store = message_store_find(store, tick);
        if (store == NULL)
        {
            store = message_store_allocate(tick);
            store->next = data->store;
            data->store = store;
        }
    }

    return store;
}

void node_print_field_value(Node* node, FieldType type, FieldValue value)
{
    switch (type)
    {
        case FIELD_INTEGER:
            repl_print("%d,%d,%d %d\n",
                node->location.x,
                node->location.y,
                node->location.z,
                value.integer);
            break;

        case FIELD_DIRECTION:
            repl_print("%d,%d,%d %s\n",
                node->location.x,
                node->location.y,
                node->location.z,
                Directions[value.direction]);
            break;

        case FIELD_STRING:
            repl_print("%d,%d,%d \"%s\"\n",
                node->location.x,
                node->location.y,
                node->location.z,
                value.string ? value.string : "");
            break;
    }
}

void node_print_field(Field* field, FieldValue value)
{
    switch (field->type)
    {
        case FIELD_INTEGER:
            repl_print(" %s:%d",field->name, value.integer);
            break;

        case FIELD_DIRECTION:
            repl_print(" %s:%s", field->name, Directions[value.direction]);
            break;

        case FIELD_STRING:
            repl_print(" %s:\"%s\"", field->name, value.string ? value.string : "");
            break;
    }
}

void node_print(Node* node)
{
    NodeData* data = node->data;
    repl_print("%d,%d,%d %s",
           node->location.x,
           node->location.y,
           node->location.z,
           data->type->name);

    for (unsigned int i = 0; i < data->type->fields->count; i++)
    {
        node_print_field(
            data->type->fields->data + i,
            data->fields != NULL ? data->fields->data[i] : (FieldValue){0});
    }

    repl_print("\n");
}

bool node_equals(Node* n1, Node* n2)
{
    bool equal = (n1->data == n2->data);
    assert(equal == location_equals(n1->location, n2->location));
    return equal;
}

NodeTree* node_tree_allocate(unsigned int level, NodeTree* parent)
{
    NodeTree* tree = calloc(1, sizeof(NodeTree));
    CHECK_OOM(tree);
    tree->level = level;
    tree->parent = parent;
    return tree;
}

static NodeLeaf* node_leaf_allocate(NodeTree* parent)
{
    NodeLeaf* leaf = calloc(1, sizeof(NodeLeaf));
    CHECK_OOM(leaf);
    leaf->parent = parent;
    return leaf;
}

void node_data_free(NodeData* data)
{
    message_store_free(data->store);
    free(data->last_input);

    if (data->fields != NULL)
    {
        for (unsigned int i = 0; i < data->type->fields->count; i++)
        {
            Field* field = data->type->fields->data + i;
            if (field->type == FIELD_STRING)
                free(data->fields->data[i].string);
        }
        free(data->fields);
    }
}

void node_tree_free(NodeTree* tree)
{
    if (tree->level != 0)
    {
        for (unsigned int i = 0; i < TREE_SIZE; i++)
        {
            if (tree->data.children[i] != NULL)
                node_tree_free(tree->data.children[i]);
        }
    }
    else
    {
        for (unsigned int i = 0; i < TREE_SIZE; i++)
        {
            NodeLeaf* leaf = tree->data.leaves[i];
            if (leaf != NULL)
            {
                for (unsigned int i = 0; i < LEAF_SIZE; i++)
                {
                    if (leaf->data[i].type != NULL)
                        node_data_free(leaf->data + i);
                }
                free(leaf);
            }
        }
    }
    free(tree);
}

#define MOST_SIGNIFICANT_BIT(NUM) (NUM != 0 ? (sizeof(NUM) * CHAR_BIT) - __builtin_clz(NUM) : 0)
NodeTree* node_tree_ensure_depth(NodeTree* tree, Location location)
{
    unsigned int x_abs = abs(location.x);
    unsigned int y_abs = abs(location.y);
    unsigned int z_abs = abs(location.z);
    unsigned int max = x_abs > y_abs ? x_abs : y_abs;
    max = max > z_abs ? max : z_abs;
    max /= LEAF_WIDTH;
    unsigned int depth = MOST_SIGNIFICANT_BIT(max);
    while (depth > tree->level)
    {
        NodeTree* children[TREE_SIZE];
        memcpy(&children, tree->data.children, sizeof(NodeTree*) * TREE_SIZE);
        for (unsigned int i = 0; i < TREE_SIZE; i++)
        {
            NodeTree* new = node_tree_allocate(tree->level, tree);
            new->data.children[(TREE_SIZE - 1) - i] = children[i];
            if (children[i] != NULL)
                children[i]->parent = new;
            tree->data.children[i] = new;
        }
        tree->level++;
    }

    return tree;
}

void node_tree_get(NodeTree* tree, Location location, Node* node, bool create)
{
    unsigned int offset = (location.x < 0 ? 1 : 0) + (location.y < 0 ? 2 : 0) + (location.z < 0 ? 4 : 0);
    if (tree->level != 0)
    {
        NodeTree* sub_tree = tree->data.children[offset];
        if (sub_tree == NULL)
        {
            if (!create)
            {
                *node = node_empty();
                return;
            }

            sub_tree = tree->data.children[offset] = node_tree_allocate(tree->level - 1, tree);
        }

        unsigned int shift = LEAF_WIDTH * (1 << (tree->level - 1));
        Location sub_location = location_create(
                location.x >= 0 ? location.x - shift : location.x + shift,
                location.y >= 0 ? location.y - shift : location.y + shift,
                location.z >= 0 ? location.z - shift : location.z + shift);
        node_tree_get(sub_tree, sub_location, node, create);
        node->location = location;
    }
    else
    {
        NodeLeaf* leaf = tree->data.leaves[offset];
        if (leaf == NULL)
        {
            if (!create)
            {
                *node = node_empty();
                return;
            }

            leaf = tree->data.leaves[offset] = node_leaf_allocate(tree);
        }

        Location sub_location = location_create(
                location.x >= 0 ? location.x : -(location.x + 1),
                location.y >= 0 ? location.y : -(location.y + 1),
                location.z >= 0 ? location.z : -(location.z + 1));
        assert(sub_location.x < LEAF_WIDTH &&
               sub_location.y < LEAF_WIDTH &&
               sub_location.z < LEAF_WIDTH);
        unsigned int leaf_offset = sub_location.x * LEAF_WIDTH * LEAF_WIDTH +
                                   sub_location.y * LEAF_WIDTH +
                                   sub_location.z;
        node->location = location;
        node->data = leaf->data + leaf_offset;
    }
}

NodeList* node_list_allocate(unsigned int count)
{
    NodeList* nodes = malloc(sizeof(NodeList) + (sizeof(Node) * count));
    CHECK_OOM(nodes);
    nodes->count = count;
    nodes->index = -1;
    nodes->next = NULL;
    return nodes;
}

void node_list_free(NodeList* nodes)
{
    NodeList* list = nodes;
    while (list != NULL)
    {
        NodeList* temp = list;
        list = list->next;
        free(temp);
    }
}

NodeList* node_list_flatten(NodeList* nodes)
{
    // TODO: Implement
    return nodes;
}

unsigned int node_list_add(NodeList* stack, Node* node)
{
    assert(stack->count != 0);
    assert(stack->index >= -1);

    unsigned int offset = 0;
    while ((unsigned int)(stack->index + 1) >= stack->count)
    {
        if (stack->next == NULL)
            stack->next = node_list_allocate(stack->count);
        offset += stack->count;
        stack = stack->next;
    }

    stack->nodes[++stack->index] = *node;
    return offset + stack->index;
}

void node_list_remove(NodeList* nodes, Node* node, bool remove_multiple)
{
    for (NodeList* list = nodes; list != NULL; list = list->next)
    {
        for (unsigned int i = 0; i < list->count; i++)
        {
            if (location_equals(node->location, list->nodes[i].location))
            {
                list->nodes[i] = node_empty();
                if (!remove_multiple)
                    break;
            }
        }
    }
}

static void node_list_insert_after_index(NodeList* nodes, unsigned int index, Node* node)
{
    assert(index <= nodes->count);

    NodeList* list = nodes;
    while (true)
    {
        for (int i = index; i <= list->index; i++)
        {
            if (location_equals(list->nodes[i].location, location_empty()))
            {
                list->nodes[i] = *node;
                *node = node_empty();
                return;
            }
        }

        assert(list->index >= -1);
        if ((unsigned int)(list->index + 1) < list->count)
        {
            list->index++;
            list->nodes[list->index] = *node;
            return;
        }

        if (list->next != NULL)
        {
            index = 0;
            list = list->next;
        }
        else
        {
            break;
        }
    }

    list->next = node_list_allocate(list->count);
    list = list->next;
    list->index = 0;
    list->nodes[0] = *node;
}

void node_list_insert_after(NodeList* nodes, Node* node, Node* target)
{
    for (NodeList* list = nodes; list != NULL; list = list->next)
    {
        int index = node - list->nodes;
        if (index >= 0 && index <= list->index)
        {
            node_list_insert_after_index(list, index, target);
            break;
        }
    }
}

Node* node_list_index(NodeList* nodes, unsigned int index)
{
    for (NodeList* list = nodes; list != NULL; list = list->next)
    {
        assert(list->index >= -1);
        if (list->index != -1 && index <= (unsigned int)list->index)
            return list->nodes + index;
        else if (index < list->count)
            return NULL;
        else
            index -= list->count;
    }

    return NULL;
}

