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

#define TREE_WIDTH 2
#define TREE_SIZE (TREE_WIDTH * TREE_WIDTH * TREE_WIDTH)

typedef struct {
    unsigned int count;
    FieldValue data[];
} FieldData;

typedef struct NodeData {
    Type* type;
    FieldData* fields;

    MessageStore* store;
} NodeData;

typedef struct NodeTree {
    struct NodeTree* parent;
    unsigned int level;
    NodeData* data;
    union {
        struct NodeTree* branches[TREE_SIZE];
        NodeData* leaves[TREE_SIZE];
    } children;
} NodeTree;

typedef struct {
    Location location;
    NodeData* data;
} Node;

typedef struct NodeList {
    unsigned int count;
    int index;
    struct NodeList* next;
    Node nodes[];
} NodeList;

#define NODE_IS_EMPTY(NODE) ((NODE)->data == NULL)
#define NODE_FIELD(NODE,INDEX,TYPE) (NODE)->data->fields->data[INDEX].TYPE
#define FIELD_GET(NODE,INDEX,TYPE) ((NODE)->data->fields != NULL ? NODE_FIELD(NODE,INDEX,TYPE) : 0)
#define FIELD_SET(NODE,INDEX,TYPE,VALUE) (node_initialize_fields(NODE), NODE_FIELD(NODE,INDEX,TYPE) = VALUE)

void node_data_free(NodeData* data);

Node node_empty(void);
NodeData* node_data_allocate(Type* type);
void node_initialize_fields(Node* node);
Messages* node_find_messages(Node* node, unsigned long long tick);
MessageStore* node_find_store(Node* node, unsigned long long tick);
void node_print_field_value(Node* node, FieldType type, FieldValue value);
void node_print_field(Field* field, FieldValue value);
void node_print(Node* node);
bool node_equals(Node* n1, Node* n2);

NodeTree* node_tree_allocate(NodeTree* parent, unsigned int level, NodeData* data);
void node_tree_free(NodeTree* tree);
NodeTree* node_tree_ensure_depth(NodeTree* tree, Location location);
void node_tree_get(NodeTree* tree, Location location, Node* node, bool create);
void node_tree_remove(NodeTree* tree, Location location, Node* node);

NodeList* node_list_allocate(unsigned int count);
void node_list_free(NodeList* nodes);
unsigned int node_list_add(NodeList* list, Node* node);
Node* node_list_index(NodeList* nodes, unsigned int index);
bool node_cursor_next(Cursor* cursor, Node* node);

#endif
