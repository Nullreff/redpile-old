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

static Node* node_allocate(Location location, Type* type)
{
    Node* node = calloc(1, sizeof(Node) + (type->fields->count * sizeof(FieldValue)));
    CHECK_OOM(node);
    node->location = location;
    node->type = type;
    node->store = NULL;
    node->last_input = NULL;
    node->fields.count = type->fields->count;
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
    repl_print("(%d,%d,%d) %s",
           node->location.x,
           node->location.y,
           node->location.z,
           node->type->name);

    for (int i = 0; i < node->type->fields->count; i++)
    {
        Field* field = node->type->fields->data + i;
        switch (field->type)
        {
            case FIELD_INT:
                repl_print(" %s:%d", field->name, node->fields.data[i].integer);
                break;

            case FIELD_DIRECTION:
                repl_print(" %s:%s", field->name, Directions[node->fields.data[i].direction]);
                break;
        }
    }

    repl_print("\n");
}

NodeList* node_list_allocate(void)
{
    NodeList* nodes = calloc(1, sizeof(NodeList));
    CHECK_OOM(nodes);
    return nodes;
}

void node_list_free(NodeList* nodes)
{
    Node* node;

    node = nodes->active;
    while (node != NULL)
    {
        Node* temp = node->next;
        node_free(node);
        node = temp;
    }

    node = nodes->inactive;
    while (node != NULL)
    {
        Node* temp = node->next;
        node_free(node);
        node = temp;
    }

    free(nodes);
}

Node* node_list_append(NodeList* nodes, Location location, Type* type)
{
    Node* node = node_allocate(location, type);

    if (type->behaviors->count > 0)
    {
        if (nodes->active != NULL)
        {
            node->next = nodes->active;
            nodes->active->prev = node;
        }
        nodes->active = node;
    }
    else
    {
        if (nodes->inactive != NULL)
        {
            node->next = nodes->inactive;
            nodes->inactive->prev = node;
        }
        nodes->inactive = node;
    }

    nodes->size++;
    return node;
}

void node_list_remove(NodeList* nodes, Node* node)
{
    if (node->prev == NULL)
    {
        assert(node == nodes->active || node == nodes->inactive);
        if (node == nodes->active)
            nodes->active = node->next;
        else
            nodes->inactive = node->next;
    }
    else
    {
        node->prev->next = node->next;
    }

    if (node->next != NULL)
        node->next->prev = node->prev;

    nodes->size--;
    node_free(node);
}

void node_list_move_after(NodeList* nodes, Node* node, Node* target)
{
    // Already in the right place
    if (node->next == target)
        return;

    // Remove 'target' from the list
    if (target->prev == NULL)
    {
        assert(target == nodes->active || target == nodes->inactive);
        if (target == nodes->active)
            nodes->active = target->next;
        else
            nodes->inactive = target->next;
    }
    else
    {
        target->prev->next = target->next;
    }

    if (target->next != NULL)
        target->next->prev = target->prev;

    // Add 'target' after 'node'
    target->next = node->next;
    if (node->next != NULL)
        node->next->prev = target;

    node->next = target;
    target->prev = node;
}

void node_list_print(NodeList* nodes)
{
    FOR_NODES(node, nodes->active)
    {
        node_print(node);
    }

    FOR_NODES(node, nodes->inactive)
    {
        node_print(node);
    }

    repl_print("Total: %u\n", nodes->size);
}

NodeStack* node_stack_allocate(unsigned int count)
{
    NodeStack* stack = malloc(sizeof(NodeStack) + (sizeof(Node*) * count));
    stack->index = -1;
    stack->count = count;
    return stack;
}

void node_stack_free(NodeStack* stack)
{
    free(stack);
}

int node_stack_push(NodeStack* stack, Node* node)
{
    if (stack->index + 1 >= stack->count)
        return -1;

    stack->nodes[++stack->index] = node;
    return stack->index;
}

bool node_stack_pop(NodeStack* stack)
{
    assert(stack->index > 0);
    stack->index--;
    return true;
}

Node* node_stack_index(NodeStack* stack, unsigned int index)
{
    return index < stack->count ? stack->nodes[index] : NULL;
}

Node* node_stack_first(NodeStack* stack)
{
    Node* node = node_stack_index(stack, 0);
    assert(node != NULL);
    return node;
}

