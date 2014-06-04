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

Rup* rup_allocate(void)
{
    Rup* rup = malloc(sizeof(Rup));
    CHECK_OOM(rup);
    rup->instructions = NULL;
    rup->size = 0;
    return rup;
}

void rup_free(Rup* rup)
{
    free(rup);
}

unsigned int rup_max_power(Rup* rup)
{
    unsigned int max = 0;
    FOR_RUP(rup)
    {
        if (inst->command == RUP_POWER && inst->value.power > max)
            max = inst->value.power;
    }
    return max;
}

static RupInst* rup_push(Rup* rup, RupCmd cmd, unsigned int delay, BlockNode* source, BlockNode* node)
{
    RupInst* inst = malloc(sizeof(RupInst));
    CHECK_OOM(inst);
    *inst = rup_inst_create(cmd, delay, source, node);
    inst->next = rup->instructions;
    rup->instructions = inst;
    rup->size++;
    return inst;
}

void rup_cmd_power(Rup* rup, unsigned int delay, BlockNode* source, BlockNode* node, unsigned int power)
{
    RupInst* inst = rup_push(rup, RUP_POWER, delay, source, node);
    inst->value.power = power;
}

void rup_cmd_swap(Rup* rup, unsigned int delay, BlockNode* source, BlockNode* node, BlockNode* target)
{
    RupInst* inst = rup_push(rup, RUP_SWAP, delay, source, node);
    inst->value.target = target;
}

RupInst rup_inst_create(RupCmd cmd, unsigned int delay, BlockNode* source, BlockNode* node)
{
    return (RupInst){cmd, delay, source, node, {0}, NULL};
}

void rup_inst_run(World* world, RupInst* inst)
{
    switch (inst->command)
    {
        case RUP_POWER:
            inst->node->block.power = inst->value.power;
            break;

        case RUP_SWAP:
            world_block_swap(world, &inst->node->block, &inst->value.target->block);
            break;
    }
}

void rup_inst_print(RupInst* inst)
{
    switch (inst->command)
    {
        case RUP_POWER:
            printf("POWER (%d,%d,%d) %u\n",
                inst->node->block.location.x,
                inst->node->block.location.y,
                inst->node->block.location.z,
                inst->value.power);
            break;

        case RUP_SWAP:
            printf("SWAP (%d,%d,%d) (%d,%d,%d)\n",
                inst->node->block.location.x,
                inst->node->block.location.y,
                inst->node->block.location.z,
                inst->value.target->block.location.x,
                inst->value.target->block.location.y,
                inst->value.target->block.location.z);
            break;
    }
}

Runmap* runmap_allocate(void)
{
    Runmap* runmap = calloc(1, sizeof(Runmap));
    CHECK_OOM(runmap);
    return runmap;
}

void runmap_free(Runmap* runmap)
{
    for (int i = 0; i < RUP_CMD_COUNT; i++)
    for (RupInst* inst = runmap->instructions[i]; inst != NULL;)
    {
        RupInst* temp = inst->next;
        free(inst);
        inst = temp;
    }
    free(runmap);
}

#define RUNMAP_INST(RUNMAP,INST) (RUNMAP)->instructions[(INST)->command]
void runmap_import(Runmap* runmap, Rup* rup)
{
    RupInst* next;
    for (RupInst* inst = rup->instructions; inst != NULL; inst = next)
    {
        next = inst->next;

        if (RUNMAP_INST(runmap, inst) != NULL)
            inst->next = RUNMAP_INST(runmap, inst);
        else
            inst->next = NULL;
        RUNMAP_INST(runmap, inst) = inst;

        runmap->sizes[inst->command]++;
    }

    rup->instructions = NULL;
    rup->size = 0;
}

