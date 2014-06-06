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

static RupNode* rup_push(Rup* rup, RupCmd cmd, unsigned int delay, BlockNode* source, BlockNode* target)
{
    RupNode* node = malloc(sizeof(RupNode));
    CHECK_OOM(node);

    node->inst = rup_inst_create(cmd, source);
    node->delay = delay;
    node->target = target;

    node->next = rup->nodes;
    if (rup->nodes != NULL)
        rup->nodes->prev = node;
    rup->nodes = node;

    rup->size++;
    return node;
}

void rup_cmd_power(Rup* rup, unsigned int delay, BlockNode* source, BlockNode* target, unsigned int power)
{
    RupNode* node = rup_push(rup, RUP_POWER, delay, source, target);
    node->inst.value.power = power;
}

void rup_cmd_swap(Rup* rup, unsigned int delay, BlockNode* source, BlockNode* target, Direction direction)
{
    RupNode* node = rup_push(rup, RUP_SWAP, delay, source, target);
    node->inst.value.direction = direction;
}

RupInst rup_inst_create(RupCmd cmd, BlockNode* source)
{
    return (RupInst){cmd, source, {0}};
}

unsigned int rup_inst_max_power(RupInst* found_inst)
{
    unsigned int max = 0;
    FOR_RUP_INST(found_inst)
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
            printf("POWER (%d,%d,%d) %u\n",
                inst->source->block.location.x,
                inst->source->block.location.y,
                inst->source->block.location.z,
                inst->value.power);
            break;

        case RUP_SWAP:
            printf("SWAP (%d,%d,%d) %s\n",
                inst->source->block.location.x,
                inst->source->block.location.y,
                inst->source->block.location.z,
                Directions[inst->value.direction]);
            break;
    }
}

RupQueue* rup_queue_allocate(unsigned int delay)
{
    RupQueue* queue = malloc(sizeof(RupQueue));
    CHECK_OOM(queue);
    queue->insts = malloc(sizeof(RupInst));
    *queue->insts = rup_inst_create(RUP_HALT, NULL);
    queue->size = 0;
    queue->delay = delay;
    queue->next = NULL;
    return queue;
}

void rup_queue_free(RupQueue* queue)
{
    free(queue->insts);
    free(queue);
}

RupInst* rup_queue_add(RupQueue* queue)
{
    queue->size++;

    // TODO: Pre-allocate space instead of reallocing on each add
    queue->insts = realloc(queue->insts, sizeof(RupInst) * (queue->size + 1));
    CHECK_OOM(queue->insts);
    queue->insts[queue->size] = rup_inst_create(RUP_HALT, NULL);

    return queue->insts + queue->size - 1;
}

RupInst* rup_queue_find_instructions(RupQueue* queue, unsigned int delay)
{
    for (;queue != NULL; queue = queue->next)
    {
        if (queue->delay == delay)
            return queue->insts;
    }

    return NULL;
}

// Deincrement the 'delay' value and remove those that fall below zero
void rup_queue_deincrement_delay(RupQueue** queue_ptr)
{
    RupQueue* queue = *queue_ptr;

    // Remove items at the head of the list
    while (true)
    {
        if (queue == NULL)
            return;

        if (queue->delay > 0)
        {
            queue->delay--;
            break;
        }

        *queue_ptr = queue->next;
        free(queue);
        queue = *queue_ptr;
    }

    // Remove items further in
    while (queue->next != NULL)
    {
        if (queue->next->delay == 0)
        {
            RupQueue* temp = queue->next->next;
            free(queue->next);
            queue->next = temp;
        }
        else
        {
            queue->next->delay--;
            queue = queue->next;
        }
    }
}

