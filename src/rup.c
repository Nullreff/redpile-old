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

static RupInst rup_inst_create(MessageType type, Node* node, unsigned int message)
{
    return (RupInst){{node->location, node->type}, type, message};
}

bool rup_node_equals(void* np1, void* np2)
{
    RupNode* n1 = np1;
    RupNode* n2 = np2;
    return n1->inst.type == n2->inst.type &&
           LOCATION_EQUALS(n1->inst.source.location, n2->inst.source.location) &&
           n1->inst.message == n2->inst.message &&
           n1->target == n2->target &&
           n1->tick == n2->tick;
}

RupNode* queue_push_inst(Queue* queue, MessageType type, unsigned long long tick, Node* source, Node* target, unsigned int message)
{
    RupNode* node = malloc(sizeof(RupNode));
    CHECK_OOM(node);

    node->inst = rup_inst_create(type, source, message);
    node->tick = tick;
    node->target = target;

    QueueNode* queue_node = queue_add(queue, source->location, target->location);
    queue_node->value = node;
    return node;
}

unsigned int rup_insts_size(RupInsts* insts)
{
    return insts->size;
}

RupInsts* rup_insts_clone(RupInsts* insts)
{
    size_t struct_size = RUP_INSTS_ALLOC_SIZE(insts->size);
    RupInsts* new_insts = malloc(struct_size);
    memcpy(new_insts, insts, struct_size);
    return new_insts;
}

RupInsts* rup_insts_allocate(void)
{
    RupInsts* insts = malloc(sizeof(RupInsts));
    insts->size = 0;
    return insts;
}

RupInsts* rup_insts_append(RupInsts* insts, RupInst* inst)
{
    // TODO: Pre-allocate space instead of reallocing on each add
    insts = realloc(insts, RUP_INSTS_ALLOC_SIZE(insts->size + 1));
    CHECK_OOM(insts);
    insts->data[insts->size] = *inst;
    insts->size++;
    return insts;
}

RupInsts* rup_insts_append_nodes(RupInsts* insts, Queue* messages, Location target, unsigned long long tick)
{
    Bucket* bucket = hashmap_get(messages->targetmap, target, false);
    if (bucket == NULL)
        return insts;

    QueueNode* queue_node = bucket->value;
    if (queue_node == NULL)
        return insts;


    do
    {
        RupNode* found = queue_node->value;
        if (found->tick == tick)
            insts = rup_insts_append(insts, &found->inst);
        queue_node = queue_node->next;
    }
    while (queue_node != NULL && LOCATION_EQUALS(queue_node->target, target));

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

bool rup_inst_equals(RupInst* i1, RupInst* i2)
{
    return i1->type == i2->type &&
           LOCATION_EQUALS(i1->source.location, i2->source.location);
}

void rup_inst_print(RupInst* inst)
{
    switch (inst->type)
    {
        case RUP_POWER:
            printf("POWER (%d,%d,%d) %u\n",
                inst->source.location.x,
                inst->source.location.y,
                inst->source.location.z,
                inst->message);
            break;

        case RUP_MOVE:
            printf("MOVE (%d,%d,%d) %s\n",
                inst->source.location.x,
                inst->source.location.y,
                inst->source.location.z,
                Directions[inst->message]);
            break;

        case RUP_REMOVE:
            printf("REMOVE (%d,%d,%d)\n",
                inst->source.location.x,
                inst->source.location.y,
                inst->source.location.z);
            break;
    }
}

void rup_inst_print_verbose(RupInst* inst, unsigned long long tick, Location target)
{
    printf("%llu ", tick);

    printf("(%d,%d,%d) => (%d,%d,%d) ",
        inst->source.location.x,
        inst->source.location.y,
        inst->source.location.z,
        target.x,
        target.y,
        target.z);

    switch (inst->type)
    {
        case RUP_POWER:  printf("POWER %u\n", inst->message); break;
        case RUP_MOVE:   printf("MOVE %s\n", Directions[inst->message]); break;
        case RUP_REMOVE: printf("REMOVE\n"); break;
    }
}

void rup_node_print(RupNode* node)
{
    switch (node->inst.type)
    {
        case RUP_POWER:
            printf("POWER (%d,%d,%d) %u\n",
                node->target->location.x,
                node->target->location.y,
                node->target->location.z,
                node->inst.message);
            break;

        case RUP_MOVE:
            printf("MOVE (%d,%d,%d) %s\n",
                node->target->location.x,
                node->target->location.y,
                node->target->location.z,
                Directions[node->inst.message]);
            break;

        case RUP_REMOVE:
            printf("REMOVE (%d,%d,%d)\n",
                node->target->location.x,
                node->target->location.y,
                node->target->location.z);
            break;
    }
}

void rup_node_print_verbose(RupNode* node, unsigned long long current_tick)
{
    rup_inst_print_verbose(&node->inst, node->tick - current_tick, node->target->location);
}

RupQueue* rup_queue_allocate(unsigned long long tick)
{
    RupQueue* queue = malloc(sizeof(RupQueue));
    CHECK_OOM(queue);
    queue->insts = rup_insts_allocate();
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

void rup_queue_add(RupQueue* queue, RupInst* inst)
{
    // TODO: Faster search
    for (int i = 0; i < queue->insts->size; i++)
    {
        RupInst* found_inst = queue->insts->data + i;

        if (rup_inst_equals(found_inst, inst))
        {
            *found_inst = *inst;
            return;
        }
    }

    queue->insts = rup_insts_append(queue->insts, inst);
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

