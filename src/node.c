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

NodeData* node_data_allocate(Type* type)
{
    NodeData* data = calloc(1, sizeof(NodeData));
    CHECK_OOM(data);
    data->type = type;
    return data;
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
    free(data);
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
    return location_equals(n1->location, n2->location);
}

NodeTree* node_tree_allocate(NodeTree* parent, unsigned int level, NodeData* data)
{
    NodeTree* tree = calloc(1, sizeof(NodeTree));
    CHECK_OOM(tree);
    tree->parent = parent;
    tree->level = level;
    tree->data = data;
    return tree;
}

void node_tree_free(NodeTree* tree)
{
    if (tree->level != 0)
    {
        for (unsigned int i = 0; i < TREE_SIZE; i++)
        {
            if (tree->children.branches[i] != NULL)
                node_tree_free(tree->children.branches[i]);
        }
    }

    free(tree);
}

#define MOST_SIGNIFICANT_BIT(NUM) (NUM != 0 ? (sizeof(NUM) * CHAR_BIT) - __builtin_clz(NUM) : 0)
NodeTree* node_tree_ensure_depth(NodeTree* tree, Location location)
{
    assert(tree->level != 0);

    unsigned int x_abs = abs(location.x);
    unsigned int y_abs = abs(location.y);
    unsigned int z_abs = abs(location.z);
    unsigned int max = x_abs > y_abs ? x_abs : y_abs;
    max = max > z_abs ? max : z_abs;
    unsigned int depth = MOST_SIGNIFICANT_BIT(max);
    while (depth > tree->level)
    {
        NodeTree* children[TREE_SIZE];
        memcpy(&children, tree->children.branches, sizeof(NodeTree*) * TREE_SIZE);
        for (unsigned int i = 0; i < TREE_SIZE; i++)
        {
            NodeTree* new = node_tree_allocate(tree, tree->level, NULL);
            new->children.branches[(TREE_SIZE - 1) - i] = children[i];
            if (children[i] != NULL)
                children[i]->parent = new;
            tree->children.branches[i] = new;
        }
        tree->level++;
    }

    return tree;
}

void node_tree_get_recursive(NodeTree* tree, Location l, Node* node, bool create)
{
    unsigned int offset = (l.x > 0 ? 1 : 0) + (l.y > 0 ? 2 : 0) + (l.z > 0 ? 4 : 0);
    if (tree->level != 0)
    {
        NodeTree* sub_tree = tree->children.branches[offset];
        if (sub_tree == NULL)
        {
            if (!create)
            {
                if (node->data == NULL)
                    node->data = tree->data;
                return;
            }

            sub_tree = tree->children.branches[offset] = node_tree_allocate(tree, tree->level - 1, NULL);
        }

        int shift = 1 << (tree->level - 1);
        Location sub_location = location_create(
                l.x > 0 ? l.x - shift : l.x + shift,
                l.y > 0 ? l.y - shift : l.y + shift,
                l.z > 0 ? l.z - shift : l.z + shift);
        node_tree_get_recursive(sub_tree, sub_location, node, create);
    }
    else
    {
        if (l.x > 1 || l.x < 0 || l.y > 1 || l.y < 0 || l.z > 1 || l.z < 0)
        {
            if (!create)
            {
                if (node->data == NULL)
                    node->data = tree->data;
                return;
            }

            assert(!"Call into node_tree_get without first calling node_tree_ensure_depth");
        }

        if (create && tree->children.leaves[offset] == NULL)
            tree->children.leaves[offset] = node_data_allocate(NULL);

        node->data = tree->children.leaves[offset];
    }

    if (node->data == NULL)
        node->data = tree->data;
}

void node_tree_get(NodeTree* tree, Location location, Node* node, bool create)
{
    node->data = NULL;
    node_tree_get_recursive(tree, location, node, create);
    node->location = location;
}

static NodeTree* node_tree_remove_recursive(NodeTree* tree, Location l, Node* node)
{
    if (tree == NULL)
        return NULL;

    unsigned int offset = (l.x > 0 ? 1 : 0) + (l.y > 0 ? 2 : 0) + (l.z > 0 ? 4 : 0);
    if (tree->level != 0)
    {
        int shift = 1 << (tree->level - 1);
        Location sub_location = location_create(
                l.x > 0 ? l.x - shift : l.x + shift,
                l.y > 0 ? l.y - shift : l.y + shift,
                l.z > 0 ? l.z - shift : l.z + shift);

        tree->children.branches[offset] = node_tree_remove_recursive(
                tree->children.branches[offset], sub_location, node);
    }
    else
    {
        if (l.x > 1 || l.x < 0 || l.y > 1 || l.y < 0 || l.z > 1 || l.z < 0)
            return tree;

        node->data = tree->children.leaves[offset];
        tree->children.leaves[offset] = NULL;
    }

    return tree;
}

void node_tree_remove(NodeTree* tree, Location location, Node* node)
{
    node->data = NULL;
    node_tree_remove_recursive(tree, location, node);
    node->location = location;
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

unsigned int node_list_add(NodeList* list, Node* node)
{
    assert(list->count != 0);
    assert(list->index >= -1);
    unsigned int offset = 0;
    while ((unsigned int)(list->index + 1) >= list->count)
    {
        if (list->next == NULL)
            list->next = node_list_allocate(list->count);
        offset += list->count;
        list = list->next;
    }
    list->nodes[++list->index] = *node;
    return offset + list->index;
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

void node_pool_init(NodePool* pool, unsigned int size)
{
    hashmap_init(&pool->map, size);
    pool->count = 0;
}

void node_pool_free(NodePool* pool, bool freeData)
{
    if (freeData)
        hashmap_free(&pool->map, (void (*)(void*))node_data_free);
    else
        hashmap_free(&pool->map, NULL);
}

void node_pool_add(NodePool* pool, Node* node)
{
    Bucket* bucket = hashmap_get(&pool->map, node->location, true);
    if (bucket->value == NULL)
        pool->count++;
    bucket->value = node->data;
}

void node_pool_remove(NodePool* pool, Node* node)
{
    hashmap_remove(&pool->map, node->location);
}

Cursor node_pool_iterator(NodePool* pool)
{
    return hashmap_get_iterator(&pool->map);
}

bool node_cursor_next(Cursor* cursor, Node* node)
{
    return cursor_next(cursor, &node->location, (void**)&node->data);
}

