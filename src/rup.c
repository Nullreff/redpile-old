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

unsigned int rup_max_power(Rup* rup)
{
    unsigned int max = 0;
    FOR_RUP(rup)
    {
        if (rup_node->inst.command == RUP_POWER && rup_node->inst.value.power > max)
            max = rup_node->inst.value.power;
    }
    return max;
}

static RupNode* rup_push(Rup* rup, RupCmd cmd, unsigned int delay, BlockNode* source, BlockNode* target)
{
    RupNode* node = malloc(sizeof(RupNode));
    CHECK_OOM(node);
    node->inst = rup_inst_create(cmd, delay, source, target);
    node->next = rup->nodes;
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

RupInst rup_inst_create(RupCmd cmd, unsigned int delay, BlockNode* source, BlockNode* target)
{
    return (RupInst){cmd, source, {0}};
}

void rup_inst_print(RupInst* inst)
{
    switch (inst->command)
    {
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

