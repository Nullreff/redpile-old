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
    unsigned int delay;
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
    unsigned int delay;
    struct RupQueue* next;
} RupQueue;

#define FOR_RUP(RUP) for (RupNode* rup_node = (RUP)->nodes; rup_node != NULL; rup_node = rup_node->next)
#define FOR_RUP_INST(RUP_INST) for (RupInst* inst = RUP_INST; inst->command != RUP_HALT; inst++)

Rup rup_empty(void);
void rup_free(Rup* rup);
void rup_cmd_power(Rup* rup, unsigned int delay, BlockNode* source, BlockNode* target, unsigned int power);
void rup_cmd_swap(Rup* rup, unsigned int delay, BlockNode* source, BlockNode* target, Direction direction);

RupInst rup_inst_create(RupCmd cmd, BlockNode* source);
unsigned int rup_inst_max_power(RupInst* inst);
void rup_inst_print(RupInst* inst);

RupQueue* rup_queue_allocate(unsigned int delay);
void rup_queue_free(RupQueue* queue);
RupInst* rup_queue_add(RupQueue* queue);
RupInst* rup_queue_find_instructions(RupQueue* queue, unsigned int delay);
void rup_queue_deincrement_delay(RupQueue** queue_ptr);

#endif
