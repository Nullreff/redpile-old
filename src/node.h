/* node.h - Node storage
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

#ifndef REDPILE_NODE_H
#define REDPILE_NODE_H

#include "location.h"
#include "common.h"
#include "message.h"
#include "type.h"

#define MAX_FIELDS 256

#define TREE_DEPTH 12
#define TREE_WIDTH 2
#define TREE_SIZE (TREE_WIDTH * TREE_WIDTH * TREE_WIDTH)

#define LEAF_WIDTH 16
#define LEAF_SIZE (LEAF_WIDTH * LEAF_WIDTH * LEAF_WIDTH)

typedef struct {
    unsigned int count;
    FieldValue data[];
} FieldData;

typedef struct NodeData {
    Type* type;
    FieldData* fields;

    MessageStore* store;
    Messages* last_input;
    unsigned long long last_input_tick;
} NodeData;

struct NodeLeaf;

typedef struct NodeTree {
    struct NodeTree* parent;
    unsigned int level;
    union {
        struct NodeTree* children[TREE_SIZE];
        struct NodeLeaf* leaves[TREE_SIZE];
    } data;
} NodeTree;

typedef struct NodeLeaf {
    NodeTree* parent;
    unsigned int ref_count;
    NodeData data[LEAF_SIZE];
} NodeLeaf;

typedef struct {
    Location location;
    NodeData* data;
    NodeLeaf* leaf;
} Node;

typedef struct NodeList {
    unsigned int count;
    int index;
    struct NodeList* next;
    Node nodes[];
} NodeList;

#define NODE_IS_EMPTY(NODE) ((NODE)->data == NULL)
#define FIELD_GET(NODE,INDEX,TYPE) (NODE)->data->fields->data[INDEX].TYPE
#define FIELD_SET(NODE,INDEX,TYPE,VALUE) FIELD_GET(NODE,INDEX,TYPE) = VALUE;
#define FOR_NODES(NODE,LIST)\
    for (NodeList* node_list = LIST; node_list != NULL; node_list = node_list->next)\
    for (int index = 0; index <= node_list->index; index++) {\
        Node* NODE = node_list->nodes + index;
#define FOR_NODES_END }

void node_data_free(NodeData* data);

Node node_empty(void);
Messages* node_find_messages(Node* node, unsigned long long tick);
MessageStore* node_find_store(Node* node, unsigned long long tick);
void node_print_field_value(Node* node, FieldType type, FieldValue value);
void node_print_field(Field* field, FieldValue value);
void node_print(Node* node);

NodeTree* node_tree_allocate(unsigned int level, NodeTree* parent);
void node_tree_free(NodeTree* tree);
void node_tree_get(NodeTree* tree, Location location, Node* node, bool create);
void node_tree_remove(NodeTree* tree, Node* node);

NodeList* node_list_allocate(unsigned int count);
void node_list_free(NodeList* nodes);
NodeList* node_list_flatten(NodeList* nodes);
unsigned int node_list_add(NodeList* stack, Node* node);
void node_list_move_after(NodeList* nodes, Node* node, Node* target);
Node* node_list_index(NodeList* nodes, unsigned int index);

#endif
