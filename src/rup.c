/* rup.c - Redpile update programming
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

#include <stdlib.h>
#include "rup.h"

static RupInsts* rup_insts_append(RupInsts* insts, QueueData* data)
{
    // TODO: Pre-allocate space instead of reallocing on each add
    insts = realloc(insts, RUP_INSTS_ALLOC_SIZE(insts->size + 1));
    CHECK_OOM(insts);
    insts->data[insts->size] = rup_inst_create(data);
    insts->size++;
    return insts;
}

RupInst rup_inst_create(QueueData* data)
{
    return (RupInst){{data->source.location, data->source.type}, data->type, data->message};
}

void queue_push_inst(Queue* queue, MessageType type, unsigned long long tick, Node* source, Node* target, unsigned int message)
{
    queue_add(queue, (QueueData) {
        .source.location = source->location,
        .source.type = source->type,
        .target.location = target->location,
        .target.node = target,
        .tick = tick,
        .type = type,
        .message = message
    });
}

void rup_insts_copy(RupInst* dest, RupInsts* source)
{
    memcpy(dest, source->data, sizeof(RupInst) * source->size);
}

RupInsts* rup_insts_allocate(unsigned int size)
{
    RupInsts* insts = malloc(RUP_INSTS_ALLOC_SIZE(size));
    insts->size = size;
    return insts;
}

unsigned int rup_insts_max_power(RupInsts* insts)
{
    unsigned int max = 0;
    for (int i = 0; i < insts->size; i++)
    {
        if (insts->data[i].type == RUP_POWER && insts->data[i].message > max)
            max = insts->data[i].message;
    }
    return max;
}

bool rup_insts_power_check(RupInsts* insts, Location loc, unsigned int power)
{
    for (int i = 0; i < insts->size; i++)
    {
        if (LOCATION_EQUALS(insts->data[i].source.location, loc) &&
            insts->data[i].type == RUP_POWER &&
            insts->data[i].message >= power)
            return false;
    }
    return true;
}

RupInst* rup_insts_find_move(RupInsts* insts)
{
    for (int i = 0; i < insts->size; i++)
    {
        if (insts->data[i].type == RUP_MOVE)
            return insts->data + i;
    }
    return NULL;
}

void message_type_print(MessageType type, unsigned int message)
{
    switch (type)
    {
        case RUP_POWER:  printf("POWER %u\n", message); break;
        case RUP_MOVE:   printf("MOVE %s\n", Directions[message]); break;
        case RUP_REMOVE: printf("REMOVE\n"); break;
    }
}

RupQueue* rup_queue_allocate(unsigned long long tick)
{
    RupQueue* queue = malloc(sizeof(RupQueue));
    CHECK_OOM(queue);
    queue->insts = rup_insts_allocate(0);
    queue->tick = tick;
    queue->next = NULL;
    return queue;
}

static void rup_queue_free_one(RupQueue* queue)
{
    free(queue->insts);
    free(queue);
}

void rup_queue_free(RupQueue* queue)
{
    while (queue != NULL)
    {
        RupQueue* temp = queue->next;
        rup_queue_free_one(queue);
        queue = temp;
    }
}

void rup_queue_add(RupQueue* queue, QueueData* data)
{
    queue->insts = rup_insts_append(queue->insts, data);
}

RupQueue* rup_queue_find(RupQueue* queue, unsigned long long tick)
{
    for (;queue != NULL; queue = queue->next)
    {
        if (queue->tick == tick)
            return queue;
    }

    return NULL;
}

RupInsts* rup_queue_find_instructions(RupQueue* queue, unsigned long long tick)
{
    RupQueue* found_queue = rup_queue_find(queue, tick);
    return found_queue != NULL ? found_queue->insts : NULL;
}

// Discard any queues that are older than the current tick
RupQueue* rup_queue_discard_old(RupQueue* queue, unsigned long long current_tick)
{
    RupQueue* return_queue = queue;

    // Remove items at the head of the list
    while (true)
    {
        if (queue == NULL)
            return NULL;

        if (queue->tick >= current_tick)
            break;

        return_queue = queue->next;
        rup_queue_free_one(queue);
        queue = return_queue;
    }

    // Remove items further in
    while (queue->next != NULL)
    {
        if (queue->next->tick >= current_tick)
        {
            queue = queue->next;
        }
        else
        {
            RupQueue* temp = queue->next->next;
            rup_queue_free_one(queue->next);
            queue->next = temp;
        }
    }

    return return_queue;
}

