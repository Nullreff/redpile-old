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

static RupNode* rup_push_inst(Rup* rup, RupCmd cmd, unsigned long long tick, BlockNode* source, BlockNode* target)
{
    RupNode* node = malloc(sizeof(RupNode));
    CHECK_OOM(node);

    node->inst = rup_inst_create(cmd, source);
    node->tick = tick;
    node->target = target;
    rup_push(rup, node);
    return node;
}

static bool rup_node_equals(RupNode* n1, RupNode* n2)
{
    bool equals = n1->inst.command == n2->inst.command &&
                  n1->inst.source == n2->inst.source &&
                  n1->target == n2->target &&
                  n1->tick == n2->tick;

    if (!equals)
        return false;

    switch (n1->inst.command)
    {
        case RUP_HALT:
            return true;

        case RUP_POWER:
            return n1->inst.value.power == n2->inst.value.power;

        case RUP_SWAP:
            return n1->inst.value.direction == n2->inst.value.direction;
    }
}

void rup_push(Rup* rup, RupNode* node)
{
    node->next = rup->nodes;
    node->prev = NULL;
    if (rup->nodes != NULL)
        rup->nodes->prev = node;
    rup->nodes = node;
    rup->size++;
}

void rup_merge(Rup* rup, Rup* append)
{
    // Exit early if theres nothing in the existing list
    if (rup->nodes == NULL)
    {
        rup->nodes = append->nodes;
        rup->size = append->size;
        return;
    }

    // Otherwise find ther first non-duplicate node
    RupNode* start = append->nodes;
    RupNode* node = append->nodes;
    while (true)
    {
        if (node == NULL)
            return;

        if (!rup_contains(rup, node))
        {
            node->prev = NULL;
            node = node->next;
            break;
        }

        start = node->next;
        free(node);
        node = start;
    }
    assert(start != NULL);

    // Then remove any duplicate nodes afterwards
    unsigned int count = 1;
    RupNode* end = start;
    while (node != NULL)
    {
        if (!rup_contains(rup, node))
        {
            end = node;
            node = node->next;
            count++;
        }
        else
        {
            node->next->prev = node->prev;
            node->prev->next = node->next;
            RupNode* temp = node->next;
            free(node);
            node = temp;
        }
    }
    assert(end != NULL);

    // Insert the new instructions at the begining
    end->next = rup->nodes;
    rup->nodes->prev = end;
    rup->nodes = start;
    rup->size += count;
    append->nodes = NULL;
}

bool rup_contains(Rup* rup, RupNode* node)
{
    FOR_RUP(found_node, rup)
    {
        if (rup_node_equals(found_node, node))
            return true;
    }
    return false;
}

void rup_remove_by_source(Rup* rup, BlockNode* source)
{
    RupNode* node = rup->nodes;
    while (node != NULL)
    {
        if (node->inst.source != source)
        {
            node = node->next;
            continue;
        }

        if (node->prev != NULL)
            node->prev->next = node->next;
        else
            rup->nodes = node->next;

        if (node->next != NULL)
            node->next->prev = node->prev;

        RupNode* deleted = node;
        node = node->next;
        free(deleted);
        rup->size--;
    }
}

void rup_cmd_power(Rup* rup, unsigned long long tick, BlockNode* source, BlockNode* target, unsigned int power)
{
    RupNode* node = rup_push_inst(rup, RUP_POWER, tick, source, target);
    node->inst.value.power = power;
}

void rup_cmd_swap(Rup* rup, unsigned long long tick, BlockNode* source, BlockNode* target, Direction direction)
{
    RupNode* node = rup_push_inst(rup, RUP_SWAP, tick, source, target);
    node->inst.value.direction = direction;
}

RupInst rup_inst_create(RupCmd cmd, BlockNode* source)
{
    return (RupInst){cmd, source, {0}};
}

unsigned int rup_inst_size(RupInst* insts)
{
    unsigned int size = 0;
    for (RupInst* i = insts; i->command != RUP_HALT; i++)
        size++;
    return size;
}

RupInst* rup_inst_clone(RupInst* insts, unsigned int size)
{
    RupInst* new_insts = malloc(sizeof(RupInst) * (size + 1));
    memcpy(new_insts, insts, sizeof(RupInst) * (size + 1));
    return new_insts;
}

RupInst* rup_inst_empty_allocate(void)
{
    RupInst* inst = malloc(sizeof(RupInst));
    *inst = rup_inst_create(RUP_HALT, NULL);
    return inst;
}

RupInst* rup_inst_append(RupInst* insts, unsigned int size, RupInst* inst)
{
    // TODO: Pre-allocate space instead of reallocing on each add
    insts = realloc(insts, sizeof(RupInst) * (size + 2));
    CHECK_OOM(insts);
    insts[size] = *inst;
    insts[size + 1] = rup_inst_create(RUP_HALT, NULL);
    return insts;
}

unsigned int rup_inst_max_power(RupInst* inst_list)
{
    unsigned int max = 0;
    FOR_RUP_INST(inst, inst_list)
    {
        if (inst->command == RUP_POWER && inst->value.power > max)
            max = inst->value.power;
    }
    return max;
}

bool rup_inst_contains_location(RupInst* inst_list, Location loc)
{
    FOR_RUP_INST(inst, inst_list)
    {
        if (location_equals(inst->source->block.location, loc))
            return true;
    }
    return false;
}

bool rup_inst_equals(RupInst* i1, RupInst* i2)
{
    return i1->command == i2->command && i1->source == i2->source;
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

void rup_node_print(RupNode* node)
{
    switch (node->inst.command)
    {
        case RUP_HALT:
            printf("HALT");
            break;

        case RUP_POWER:
            printf("POWER (%d,%d,%d) %u\n",
                node->target->block.location.x,
                node->target->block.location.y,
                node->target->block.location.z,
                node->inst.value.power);
            break;

        case RUP_SWAP:
            printf("SWAP (%d,%d,%d) %s\n",
                node->target->block.location.x,
                node->target->block.location.y,
                node->target->block.location.z,
                Directions[node->inst.value.direction]);
            break;
    }
}

void rup_node_print_verbose(RupNode* node)
{
    switch (node->inst.command)
    {
        case RUP_HALT:
            printf("HALT");
            break;

        case RUP_POWER:
            printf("%llu POWER (%d,%d,%d) -> (%d,%d,%d) %u\n",
                node->tick,
                node->inst.source->block.location.x,
                node->inst.source->block.location.y,
                node->inst.source->block.location.z,
                node->target->block.location.x,
                node->target->block.location.y,
                node->target->block.location.z,
                node->inst.value.power);
            break;

        case RUP_SWAP:
            printf("%llu SWAP (%d,%d,%d) -> (%d,%d,%d) %s\n",
                node->tick,
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
    queue->next = NULL;
    return queue;
}

void rup_queue_free(RupQueue* queue)
{
    free(queue->insts);
    free(queue);
}

RupInst* rup_queue_find_inst(RupQueue* queue, RupInst* inst)
{
    // TODO: Faster search
    FOR_RUP_INST(found_inst, queue->insts)
    {
        if (rup_inst_equals(found_inst, inst))
            return found_inst;
    }
    return NULL;
}

void rup_queue_add(RupQueue* queue, RupInst* inst)
{
    queue->insts = rup_inst_append(queue->insts, queue->size++, inst);
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
        rup_queue_free(queue);
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
            rup_queue_free(queue->next);
            queue->next = temp;
        }
    }

    return return_queue;
}

