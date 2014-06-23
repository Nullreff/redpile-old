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

static RupNode* rup_push_inst(Rup* rup, RupCmd cmd, unsigned long long tick, Node* source, Node* target)
{
    RupNode* node = malloc(sizeof(RupNode));
    CHECK_OOM(node);

    node->inst = rup_inst_create(cmd, source);
    node->tick = tick;
    node->target = target;

    rup_push(rup, node);
    return node;
}

static bool rup_node_type_equals(RupNode* n1, RupNode* n2)
{
    return n1->inst.command == n2->inst.command &&
           LOCATION_EQUALS(n1->inst.source, n2->inst.source) &&
           n1->target == n2->target &&
           n1->tick == n2->tick;
}

static bool rup_node_equals(RupNode* n1, RupNode* n2)
{
    if (!rup_node_type_equals(n1, n2))
        return false;

    switch (n1->inst.command)
    {
        case RUP_POWER:
            return n1->inst.value.power == n2->inst.value.power;

        case RUP_MOVE:
            return n1->inst.value.direction == n2->inst.value.direction;

        case RUP_REMOVE:
            return true;

        default:
            ERROR("Unknown RUP command\n");
    }
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
        Bucket* bucket = hashmap_get(rup->sourcemap, node->inst.source, true);
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

void rup_cmd_power(Rup* rup, unsigned long long tick, Node* source, Node* target, unsigned int power)
{
    RupNode* node = rup_push_inst(rup, RUP_POWER, tick, source, target);
    node->inst.value.power = power;
}

void rup_cmd_move(Rup* rup, unsigned long long tick, Node* source, Node* target, Direction direction)
{
    RupNode* node = rup_push_inst(rup, RUP_MOVE, tick, source, target);
    node->inst.value.direction = direction;
}

void rup_cmd_remove(Rup* rup, unsigned long long tick, Node* source, Node* target)
{
    rup_push_inst(rup, RUP_REMOVE, tick, source, target);
}

RupInst rup_inst_create(RupCmd cmd, Node* node)
{
    return (RupInst){cmd, node->location, node->type, {0}};
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

RupInsts* rup_insts_append_nodes(RupInsts* insts, Rup* messages, Location target)
{
    Bucket* bucket = hashmap_get(messages->targetmap, target, false);
    if (bucket == NULL)
        return insts;

    RupNode* found = bucket->value;
    if (found == NULL)
        return insts;

    do
    {
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
        if (insts->data[i].command == RUP_POWER && insts->data[i].value.power > max)
            max = insts->data[i].value.power;
    }
    return max;
}

bool rup_insts_power_check(RupInsts* insts, Location loc, unsigned int power)
{
    for (int i = 0; i < insts->size; i++)
    {
        if (LOCATION_EQUALS(insts->data[i].source, loc) &&
            insts->data[i].command == RUP_POWER &&
            insts->data[i].value.power >= power)
            return false;
    }
    return true;
}

RupInst* rup_insts_find_move(RupInsts* insts)
{
    for (int i = 0; i < insts->size; i++)
    {
        if (insts->data[i].command == RUP_MOVE)
            return insts->data + i;
    }
    return NULL;
}

bool rup_inst_equals(RupInst* i1, RupInst* i2)
{
    return i1->command == i2->command && LOCATION_EQUALS(i1->source, i2->source);
}

void rup_inst_print(RupInst* inst)
{
    switch (inst->command)
    {
        case RUP_POWER:
            printf("POWER (%d,%d,%d) %u\n",
                inst->source.x,
                inst->source.y,
                inst->source.z,
                inst->value.power);
            break;

        case RUP_MOVE:
            printf("MOVE (%d,%d,%d) %s\n",
                inst->source.x,
                inst->source.y,
                inst->source.z,
                Directions[inst->value.direction]);
            break;

        case RUP_REMOVE:
            printf("REMOVE (%d,%d,%d)\n",
                inst->source.x,
                inst->source.y,
                inst->source.z);
            break;
    }
}

void rup_node_print(RupNode* node)
{
    switch (node->inst.command)
    {
        case RUP_POWER:
            printf("POWER (%d,%d,%d) %u\n",
                node->target->location.x,
                node->target->location.y,
                node->target->location.z,
                node->inst.value.power);
            break;

        case RUP_MOVE:
            printf("MOVE (%d,%d,%d) %s\n",
                node->target->location.x,
                node->target->location.y,
                node->target->location.z,
                Directions[node->inst.value.direction]);
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
    if (node->tick > current_tick)
        printf("%llu ", node->tick - current_tick);
    else if (node->tick < current_tick)
        printf("-%llu ", current_tick - node->tick);

    printf("(%d,%d,%d) => (%d,%d,%d) ",
        node->inst.source.x,
        node->inst.source.y,
        node->inst.source.z,
        node->target->location.x,
        node->target->location.y,
        node->target->location.z);

    switch (node->inst.command)
    {
        case RUP_POWER:  printf("POWER %u\n", node->inst.value.power); break; 
        case RUP_MOVE:   printf("MOVE %s\n", Directions[node->inst.value.direction]); break;
        case RUP_REMOVE: printf("REMOVE\n"); break;
    }
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

