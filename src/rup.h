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

#include "block.h"

typedef enum {
    RUP_HALT = 0,
    RUP_POWER = 1,
    RUP_SWAP  = 2,
} RupCmd;

typedef struct {
    RupCmd command;
    BlockNode* source;
    union {
        unsigned int power;
        Direction direction;
    } value;
} RupInst;

typedef struct RupNode {
    RupInst inst;
    BlockNode* target;
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
    bool executed;
    struct RupQueue* next;
} RupQueue;

#define FOR_RUP(NODE,RUP) for (RupNode* (NODE) = (RUP)->nodes; (NODE) != NULL; (NODE) = (NODE)->next)
#define FOR_RUP_INST(INST,LIST) for (RupInst* (INST) = (LIST); (INST)->command != RUP_HALT; (INST)++)

Rup rup_empty(void);
void rup_free(Rup* rup);
void rup_cmd_power(Rup* rup, unsigned long long tick, BlockNode* source, BlockNode* target, unsigned int power);
void rup_cmd_swap(Rup* rup, unsigned long long tick, BlockNode* source, BlockNode* target, Direction direction);

RupInst rup_inst_create(RupCmd cmd, BlockNode* source);
unsigned int rup_inst_max_power(RupInst* inst);
bool rup_inst_contains_location(RupInst* inst_list, Location loc);
void rup_inst_print(RupInst* node);
void rup_node_print(RupNode* node);

RupQueue* rup_queue_allocate(unsigned long long tick);
void rup_queue_free(RupQueue* queue);
bool rup_queue_add(RupQueue* queue, RupInst* inst);
RupInst* rup_queue_find_instructions(RupQueue* queue, unsigned long long tick);
RupQueue* rup_queue_find(RupQueue* queue, unsigned long long tick);
RupQueue* rup_queue_discard_old(RupQueue* queue, unsigned long long current_tick);

#endif
