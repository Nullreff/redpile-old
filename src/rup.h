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
#include "queue.h"

typedef enum {
    RUP_POWER,
    RUP_MOVE,
    RUP_REMOVE
} MessageType;

typedef struct {
    struct {
        Location location;
        Type type;
    } source;
    MessageType type;
    unsigned int message;
} RupInst;

typedef struct {
    unsigned int size;
    RupInst data[];
} RupInsts;

typedef struct RupNode {
    RupInst inst;
    Node* target;
    unsigned long long tick;
} RupNode;

typedef struct RupQueue {
    RupInsts* insts;
    unsigned long long tick;
    struct RupQueue* next;
} RupQueue;

#define RUP_INSTS_ALLOC_SIZE(SIZE) (sizeof(RupInsts) + sizeof(RupInst) * (SIZE))

RupInst rup_inst_create(QueueData* data);
void queue_push_inst(Queue* queue, MessageType type, unsigned long long tick, Node* source, Node* target, unsigned int message);

unsigned int rup_inst_size(RupInst* insts);
void rup_insts_copy(RupInst* dest, RupInsts* source);
RupInsts* rup_insts_allocate(unsigned int size);
unsigned int rup_insts_max_power(RupInsts* inst);
bool rup_insts_power_check(RupInsts* insts, Location loc, unsigned int power);
RupInst* rup_insts_find_move(RupInsts* insts);
void message_type_print(MessageType type, unsigned int message);

RupQueue* rup_queue_allocate(unsigned long long tick);
void rup_queue_free(RupQueue* queue);
void rup_queue_add(RupQueue* queue, QueueData* data);
RupInsts* rup_queue_find_instructions(RupQueue* queue, unsigned long long tick);
RupQueue* rup_queue_find(RupQueue* queue, unsigned long long tick);
RupQueue* rup_queue_discard_old(RupQueue* queue, unsigned long long current_tick);

#endif
