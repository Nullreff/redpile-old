/* rup.h - Redpile update programming language
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

#ifndef REDPILE_RUP_H
#define REDPILE_RUP_H

#include "node.h"
#include "hashmap.h"

typedef enum {
    RUP_POWER,
    RUP_MOVE,
    RUP_REMOVE
} RupCmd;

typedef struct {
    RupCmd command;
    Location source;
    Material source_material;
    union {
        unsigned int power;
        Direction direction;
    } value;
} RupInst;

typedef struct {
    unsigned int size;
    RupInst data[];
} RupInsts;

typedef struct RupNode {
    RupInst inst;
    Node* target;
    unsigned long long tick;
    struct RupNode* next;
    struct RupNode* prev;
} RupNode;

typedef struct {
    RupNode* nodes;
    Hashmap* targetmap;
    Hashmap* sourcemap;
} Rup;

typedef struct {
    unsigned int size;
    RupNode* data[];
} RupNodeList;

typedef struct RupQueue {
    RupInsts* insts;
    unsigned long long tick;
    struct RupQueue* next;
} RupQueue;

#define FOR_RUP(NODE,RUP) for (RupNode* (NODE) = (RUP)->nodes; (NODE) != NULL; (NODE) = (NODE)->next)
#define RUP_INSTS_ALLOC_SIZE(SIZE) (sizeof(RupInsts) + sizeof(RupInst) * (SIZE))

Rup rup_empty(bool track_targets, bool track_sources);
void rup_free(Rup* rup);
void rup_push(Rup* rup, RupNode* node);
void rup_merge(Rup* rup, Rup* append);
bool rup_contains(Rup* rup, RupNode* node);
void rup_remove_by_source(Rup* rup, Location source);
void rup_cmd_power(Rup* rup, unsigned long long tick, Node* source, Node* target, unsigned int power);
void rup_cmd_move(Rup* rup, unsigned long long tick, Node* source, Node* target, Direction direction);
void rup_cmd_remove(Rup* rup, unsigned long long tick, Node* source, Node* target);

RupInst rup_inst_create(RupCmd cmd, Node* node);
unsigned int rup_inst_size(RupInst* insts);
RupInsts* rup_insts_clone(RupInsts* source);
RupInsts* rup_insts_allocate(void);
RupInsts* rup_insts_append(RupInsts* insts, RupInst* inst);
unsigned int rup_insts_max_power(RupInsts* inst);
bool rup_insts_power_check(RupInsts* insts, Location loc, unsigned int power);
RupInst* rup_insts_find_move(RupInsts* insts);
void rup_inst_print(RupInst* node);
void rup_node_print(RupNode* node);
void rup_node_print_verbose(RupNode* node);

RupQueue* rup_queue_allocate(unsigned long long tick);
void rup_queue_free(RupQueue* queue);
void rup_queue_add(RupQueue* queue, RupInst* inst);
RupInsts* rup_queue_find_instructions(RupQueue* queue, unsigned long long tick);
RupQueue* rup_queue_find(RupQueue* queue, unsigned long long tick);
RupQueue* rup_queue_discard_old(RupQueue* queue, unsigned long long current_tick);

#endif
