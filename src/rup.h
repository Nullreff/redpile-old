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

typedef enum {
    RUP_HALT,
    RUP_POWER,
    RUP_MOVE,
    RUP_REMOVE
} RupCmd;

typedef struct {
    RupCmd command;
    Node* source;
    union {
        unsigned int power;
        Direction direction;
    } value;
} RupInst;

typedef struct RupNode {
    RupInst inst;
    Node* target;
    unsigned long long tick;
    struct RupNode* next;
    struct RupNode* prev;
} RupNode;

typedef struct {
    RupNode* nodes;
    unsigned int size;
} Rup;

typedef struct RupQueue {
    RupInst* insts;
    unsigned int size;
    unsigned long long tick;
    struct RupQueue* next;
} RupQueue;

#define FOR_RUP(NODE,RUP) for (RupNode* (NODE) = (RUP)->nodes; (NODE) != NULL; (NODE) = (NODE)->next)
#define FOR_RUP_INST(INST,LIST) for (RupInst* (INST) = (LIST); (INST)->command != RUP_HALT; (INST)++)

Rup rup_empty(void);
void rup_free(Rup* rup);
void rup_push(Rup* rup, RupNode* node);
void rup_merge(Rup* rup, Rup* append);
bool rup_contains(Rup* rup, RupNode* node);
void rup_remove_by_source(Rup* rup, Node* source);
void rup_cmd_power(Rup* rup, unsigned long long tick, Node* source, Node* target, unsigned int power);
void rup_cmd_move(Rup* rup, unsigned long long tick, Node* source, Node* target, Direction direction);
void rup_cmd_remove(Rup* rup, unsigned long long tick, Node* source, Node* target);

RupInst rup_inst_create(RupCmd cmd, Node* source);
unsigned int rup_inst_size(RupInst* insts);
RupInst* rup_inst_clone(RupInst* source, unsigned int size);
RupInst* rup_inst_empty_allocate(void);
RupInst* rup_inst_append(RupInst* insts, unsigned int size, RupInst* inst);
unsigned int rup_inst_max_power(RupInst* inst);
bool rup_inst_contains_location(RupInst* inst_list, Location loc);
bool rup_inst_contains_power(RupInst* inst_list, Location loc);
RupInst* rup_inst_find_move(RupInst* inst_list);
void rup_inst_print(RupInst* node);
void rup_node_print(RupNode* node);
void rup_node_print_verbose(RupNode* node);

RupQueue* rup_queue_allocate(unsigned long long tick);
void rup_queue_free(RupQueue* queue);
void rup_queue_add(RupQueue* queue, RupInst* inst);
RupInst* rup_queue_find_instructions(RupQueue* queue, unsigned long long tick);
RupQueue* rup_queue_find(RupQueue* queue, unsigned long long tick);
RupQueue* rup_queue_discard_old(RupQueue* queue, unsigned long long current_tick);

#endif
