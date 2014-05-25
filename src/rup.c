/* rup.c - Redstone update programming language
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

Rup* rup_allocate(void)
{
    Rup* rup = malloc(sizeof(Rup));
    rup->instructions = NULL;
    return rup;
}

void rup_free(Rup* rup)
{
    RupInst* inst = rup->instructions;
    while (inst != NULL)
    {
        RupInst* temp = inst->next;
        free(inst);
        inst = temp;
    }
    free(rup);
}

static RupInst* rup_push(Rup* rup, RupCmd cmd, Block* block)
{
    RupInst* inst = malloc(sizeof(RupInst));
    *inst = rup_inst_create(cmd, block);
    inst->next = rup->instructions;
    rup->instructions = inst;
    return inst;
}

void rup_cmd_power(Rup* rup, Block* block, unsigned int power)
{
    RupInst* inst = rup_push(rup, RUP_POWER, block);
    inst->value.power = power;
}

void rup_cmd_state(Rup* rup, Block* block, unsigned int state)
{
    RupInst* inst = rup_push(rup, RUP_STATE, block);
    inst->value.state = state;
}

void rup_cmd_move(Rup* rup, Block* block, Block* target)
{
    RupInst* inst = rup_push(rup, RUP_MOVE, block);
    inst->value.target = target;
}

void rup_cmd_set(Rup* rup, Block* block, Material material)
{
    RupInst* inst = rup_push(rup, RUP_SET, block);
    inst->value.material = material;
}

RupInst rup_inst_create(RupCmd cmd, Block* block)
{
    return (RupInst){cmd, block->location, block, {0}, NULL};
}

void rup_inst_run(RupInst* inst)
{
    switch (inst->command)
    {
        case RUP_POWER:
            inst->block->power = inst->value.power;
            inst->block->updated = true;
            break;

        case RUP_STATE:
            inst->block->power_state = inst->value.state;
            break;

        case RUP_MOVE:
            block_move(inst->block, inst->value.target);
            break;

        case RUP_SET:
            inst->block->material = inst->value.material;
            break;
    }
}

void rup_inst_print(RupInst* inst)
{
    switch (inst->command)
    {
        case RUP_POWER:
            printf("POWER (%d,%d,%d) %u\n",
                inst->block->location.x,
                inst->block->location.y,
                inst->block->location.z,
                inst->value.power);
            break;

        case RUP_STATE:
            printf("STATE (%d,%d,%d) %u\n",
                inst->block->location.x,
                inst->block->location.y,
                inst->block->location.z,
                inst->value.state);
            break;

        case RUP_MOVE:
            printf("MOVE (%d,%d,%d) (%d,%d,%d)\n",
                inst->block->location.x,
                inst->block->location.y,
                inst->block->location.z,
                inst->value.target->location.x,
                inst->value.target->location.y,
                inst->value.target->location.z);
            break;

        case RUP_SET:
            printf("SET (%d,%d,%d) %s\n",
                inst->block->location.x,
                inst->block->location.y,
                inst->block->location.z,
                Materials[inst->value.material]);
            break;
    }
}

RupList* rup_list_allocate(unsigned int size)
{
    RupList* list = malloc(sizeof(RupList));
    list->rups = malloc(size * sizeof(Rup*));
    list->size = size;
    for (int i = 0; i < size; i++)
    {
        list->rups[i] = rup_allocate();
    }
    return list;
}

void rup_list_free(RupList* list)
{
    for (int i = 0; i < list->size; i++)
    {
        if (list->rups + i != NULL)
            rup_free(list->rups[i]);
    }
    free(list->rups);
    free(list);
}

Runmap* runmap_allocate(void)
{
    return calloc(1, sizeof(Runmap));
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
    }

    rup->instructions = NULL;
}

