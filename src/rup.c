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

static bool rup_node_equals(RupNode* n1, RupNode* n2)
{
    return n1->inst.type == n2->inst.type &&
           LOCATION_EQUALS(n1->inst.source.location, n2->inst.source.location) &&
           n1->inst.message == n2->inst.message &&
           n1->target == n2->target &&
           n1->tick == n2->tick;
}

Rup rup_empty(bool track_targets, bool track_sources, unsigned int size)
{
    Hashmap* targetmap = track_targets ? hashmap_allocate(size) : NULL;
    Hashmap* sourcemap = track_sources ? hashmap_allocate(size) : NULL;
    return (Rup){NULL, targetmap, sourcemap};
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

    if (rup->targetmap != NULL)
        hashmap_free(rup->targetmap, NULL);
    if (rup->sourcemap != NULL)
        hashmap_free(rup->sourcemap, free);
}

void rup_push(Rup* rup, RupNode* node)
{
    if (rup->targetmap != NULL)
    {
        // Add the node to the list by it's target
        Bucket* bucket = hashmap_get(rup->targetmap, node->target->location, true);
        if (bucket->value == NULL)
        {
            // No nodes with this source exist yet
            // Stick it in the front of the list
            node->next = rup->nodes;
            node->prev = NULL;
            if (rup->nodes != NULL)
                rup->nodes->prev = node;
            rup->nodes = node;
            bucket->value = node;
        }
        else
        {
            // Otherwise stick it after the existing one
            RupNode* existing = bucket->value;
            node->next = existing->next;
            node->prev = existing;
            if (existing->next != NULL)
                existing->next->prev = node;
            existing->next = node;
        }
    }
    else
    {
        node->next = rup->nodes;
        node->prev = NULL;
        if (rup->nodes != NULL)
            rup->nodes->prev = node;
        rup->nodes = node;
    }

    if (rup->sourcemap != NULL)
    {
        // Add a reference to it by source
        Bucket* bucket = hashmap_get(rup->sourcemap, node->inst.source.location, true);
        if (bucket->value == NULL)
        {
            RupNodeList* source_list = malloc(sizeof(RupNodeList) +  sizeof(RupNode));
            CHECK_OOM(source_list);
            source_list->size = 1;
            source_list->nodes[0] = node;
            bucket->value = source_list;
        }
        else
        {
            RupNodeList* source_list = bucket->value;
            unsigned int new_size = source_list->size + 1;

            // TODO: Pre-allocate space instead of reallocing on each add
            source_list = realloc(source_list, sizeof(RupNodeList) + (sizeof(RupNode) * new_size));
            CHECK_OOM(source_list);
            source_list->size = new_size;
            source_list->nodes[new_size - 1] = node;
            bucket->value = source_list;
        }
    }
}

RupNode* rup_push_inst(Rup* rup, MessageType type, unsigned long long tick, Node* source, Node* target, unsigned int message)
{
    RupNode* node = malloc(sizeof(RupNode));
    CHECK_OOM(node);

    node->inst = rup_inst_create(type, source, message);
    node->tick = tick;
    node->target = target;

    rup_push(rup, node);
    return node;
}

void rup_merge(Rup* rup, Rup* append)
{
    // Exit early if theres nothing to append
    if (append->nodes == NULL)
        return;

    // Merge in any that haven't already been added
    RupNode* node = append->nodes;
    while (node != NULL)
    {
        RupNode* temp = node->next;
        if (!rup_contains(rup, node))
            rup_push(rup, node);
        else
            free(node);
        node = temp;
    }

    append->nodes = NULL;
}

bool rup_contains(Rup* rup, RupNode* node)
{
    Bucket* bucket = hashmap_get(rup->targetmap, node->target->location, false);
    if (bucket == NULL)
        return false;

    RupNode* found = bucket->value;
    if (found == NULL)
        return false;

    do
    {
        if (rup_node_equals(found, node))
            return true;
        found = found->next;
    }
    while (found != NULL && LOCATION_EQUALS(found->target->location, node->target->location));

    return false;
}

static void rup_remove(Rup* rup, RupNode* node)
{
    if (rup->targetmap != NULL)
    {
        Bucket* bucket = hashmap_get(rup->targetmap, node->target->location, false);
        assert(bucket != NULL);

        if (bucket->value == node)
        {
            if (node->next != NULL && LOCATION_EQUALS(node->target->location, node->next->target->location))
                // Change the hashmap to target the next node
                bucket->value = node->next;
            else
                // We ran out of nodes with this target
                bucket->value = NULL;
        }
    }

    if (node->prev != NULL)
        node->prev->next = node->next;
    else
        rup->nodes = node->next;

    if (node->next != NULL)
        node->next->prev = node->prev;

    free(node);
}

void rup_remove_by_source(Rup* rup, Location source)
{
    Bucket* bucket = hashmap_get(rup->sourcemap, source, false);
    if (bucket == NULL)
        return;

    RupNodeList* node_list = bucket->value;
    for (int i = 0; i < node_list->size; i++)
        rup_remove(rup, node_list->nodes[i]);

    node_list->size = 0;
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

RupInsts* rup_insts_append_nodes(RupInsts* insts, Rup* messages, Location target, unsigned long long tick)
{
    Bucket* bucket = hashmap_get(messages->targetmap, target, false);
    if (bucket == NULL)
        return insts;

    RupNode* found = bucket->value;
    if (found == NULL)
        return insts;

    do
    {
        if (found->tick == tick)
            insts = rup_insts_append(insts, &found->inst);
        found = found->next;
    }
    while (found != NULL && LOCATION_EQUALS(found->target->location, target));

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

