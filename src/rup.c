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

Rup rup_empty(void)
{
    return (Rup){NULL, 0};
}

void rup_free(Rup* rup)
{
    RupNode* node = rup->nodes;
    while (node != NULL)
    {
        RupNode* temp = node->next;
        free(node);
        node = temp;
    }
}

static RupNode* rup_push(Rup* rup, RupCmd cmd, unsigned long long tick, BlockNode* source, BlockNode* target)
{
    RupNode* node = malloc(sizeof(RupNode));
    CHECK_OOM(node);

    node->inst = rup_inst_create(cmd, source);
    node->tick = tick;
    node->target = target;

    node->next = rup->nodes;
    if (rup->nodes != NULL)
        rup->nodes->prev = node;
    rup->nodes = node;

    rup->size++;
    return node;
}

void rup_cmd_power(Rup* rup, unsigned long long tick, BlockNode* source, BlockNode* target, unsigned int power)
{
    RupNode* node = rup_push(rup, RUP_POWER, tick, source, target);
    node->inst.value.power = power;
}

void rup_cmd_swap(Rup* rup, unsigned long long tick, BlockNode* source, BlockNode* target, Direction direction)
{
    RupNode* node = rup_push(rup, RUP_SWAP, tick, source, target);
    node->inst.value.direction = direction;
}

RupInst rup_inst_create(RupCmd cmd, BlockNode* source)
{
    return (RupInst){cmd, source, {0}};
}

unsigned int rup_inst_max_power(RupInst* found_inst)
{
    unsigned int max = 0;
    FOR_RUP_INST(inst, found_inst)
    {
        if (inst->command == RUP_POWER && inst->value.power > max)
            max = inst->value.power;
    }
    return max;
}

void rup_inst_print(RupInst* inst)
{
    switch (inst->command)
    {
        case RUP_HALT:
            printf("HALT");
            break;

        case RUP_POWER:
            printf("POWER %u from (%d,%d,%d)\n",
                inst->value.power,
                inst->source->block.location.x,
                inst->source->block.location.y,
                inst->source->block.location.z);
            break;

        case RUP_SWAP:
            printf("SWAP %s from (%d,%d,%d)\n",
                Directions[inst->value.direction],
                inst->source->block.location.x,
                inst->source->block.location.y,
                inst->source->block.location.z);
            break;
    }
}

void rup_node_print(RupNode* node)
{
    switch (node->inst.command)
    {
        case RUP_HALT:
            printf("HALT");
            break;

        case RUP_POWER:
            printf("POWER (%d,%d,%d) -> (%d,%d,%d) %u\n",
                node->inst.source->block.location.x,
                node->inst.source->block.location.y,
                node->inst.source->block.location.z,
                node->target->block.location.x,
                node->target->block.location.y,
                node->target->block.location.z,
                node->inst.value.power);
            break;

        case RUP_SWAP:
            printf("SWAP (%d,%d,%d) -> (%d,%d,%d) %s\n",
                node->inst.source->block.location.x,
                node->inst.source->block.location.y,
                node->inst.source->block.location.z,
                node->target->block.location.x,
                node->target->block.location.y,
                node->target->block.location.z,
                Directions[node->inst.value.direction]);
            break;
    }
}

RupQueue* rup_queue_allocate(unsigned long long tick)
{
    RupQueue* queue = malloc(sizeof(RupQueue));
    CHECK_OOM(queue);
    queue->insts = malloc(sizeof(RupInst));
    *queue->insts = rup_inst_create(RUP_HALT, NULL);
    queue->size = 0;
    queue->tick = tick;
    queue->executed = false;
    queue->next = NULL;
    return queue;
}

void rup_queue_free(RupQueue* queue)
{
    free(queue->insts);
    free(queue);
}

void rup_queue_add(RupQueue* queue, RupInst* inst)
{
    queue->size++;

    // TODO: Pre-allocate space instead of reallocing on each add
    queue->insts = realloc(queue->insts, sizeof(RupInst) * (queue->size + 1));
    CHECK_OOM(queue->insts);
    queue->insts[queue->size] = rup_inst_create(RUP_HALT, NULL);

    RupInst* new_inst = queue->insts + queue->size - 1;
    memcpy(new_inst, inst, sizeof(RupInst));
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

RupInst* rup_queue_find_instructions(RupQueue* queue, unsigned long long tick)
{
    RupQueue* found_queue = rup_queue_find(queue, tick);
    return found_queue != NULL ? found_queue->insts : NULL;
}

// Discard any queues that are older than the current tick
void rup_queue_discard_old(RupQueue** queue_ptr, unsigned long long current_tick)
{
    RupQueue* queue = *queue_ptr;

    // Remove items at the head of the list
    while (true)
    {
        if (queue == NULL)
            return;

        if (queue->tick >= current_tick)
            break;

        *queue_ptr = queue->next;
        free(queue);
        queue = *queue_ptr;
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
            free(queue->next);
            queue->next = temp;
        }
    }
}

