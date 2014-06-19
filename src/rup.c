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
           n1->inst.source == n2->inst.source &&
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
    // Exit early if theres nothing in one of the lists
    if (append->nodes == NULL)
        return;

    if (rup->nodes == NULL)
    {
        rup->nodes = append->nodes;
        rup->size = append->size;
        return;
    }

    // Otherwise find the first non-duplicate node
    RupNode* start = append->nodes;
    RupNode* node = append->nodes;
    while (true)
    {
        if (!rup_contains(rup, node))
        {
            node->prev = NULL;
            node = node->next;
            break;
        }

        start = node->next;
        free(node);

        // Reached the end of the list
        if (start == NULL)
            return;

        node = start;
    }
    assert(start != NULL);

    // Then remove any duplicate nodes
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

void rup_remove_by_source(Rup* rup, Node* source)
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

RupInst rup_inst_create(RupCmd cmd, Node* source)
{
    return (RupInst){cmd, source, {0}};
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
        if (location_equals(insts->data[i].source->location, loc) &&
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
    return i1->command == i2->command && i1->source == i2->source;
}

void rup_inst_print(RupInst* inst)
{
    switch (inst->command)
    {
        case RUP_POWER:
            printf("POWER (%d,%d,%d) %u\n",
                inst->source->location.x,
                inst->source->location.y,
                inst->source->location.z,
                inst->value.power);
            break;

        case RUP_MOVE:
            printf("MOVE (%d,%d,%d) %s\n",
                inst->source->location.x,
                inst->source->location.y,
                inst->source->location.z,
                Directions[inst->value.direction]);
            break;

        case RUP_REMOVE:
            printf("REMOVE (%d,%d,%d)\n",
                inst->source->location.x,
                inst->source->location.y,
                inst->source->location.z);
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

void rup_node_print_verbose(RupNode* node)
{
    switch (node->inst.command)
    {
        case RUP_POWER:
            printf("%llu POWER (%d,%d,%d) -> (%d,%d,%d) %u\n",
                node->tick,
                node->inst.source->location.x,
                node->inst.source->location.y,
                node->inst.source->location.z,
                node->target->location.x,
                node->target->location.y,
                node->target->location.z,
                node->inst.value.power);
            break;

        case RUP_MOVE:
            printf("%llu MOVE (%d,%d,%d) -> (%d,%d,%d) %s\n",
                node->tick,
                node->inst.source->location.x,
                node->inst.source->location.y,
                node->inst.source->location.z,
                node->target->location.x,
                node->target->location.y,
                node->target->location.z,
                Directions[node->inst.value.direction]);
            break;

        case RUP_REMOVE:
            printf("%llu REMOVE (%d,%d,%d) -> (%d,%d,%d)\n",
                node->tick,
                node->inst.source->location.x,
                node->inst.source->location.y,
                node->inst.source->location.z,
                node->target->location.x,
                node->target->location.y,
                node->target->location.z);
            break;
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

void rup_queue_free(RupQueue* queue)
{
    free(queue->insts);
    free(queue);
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

