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

static RupInst* rup_inst_allocate(RupCmd cmd, Block* block)
{
    RupInst* inst = malloc(sizeof(RupInst));
    inst->command = cmd;
    inst->location = block->location;
    inst->block = block;
    return inst;
}

static RupInst* rup_push(Rup* rup, RupCmd cmd, Block* block)
{
    RupInst* inst = rup_inst_allocate(cmd, block);
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

void rup_run(RupInst* inst)
{
    switch (inst->command)
    {
        case RUP_POWER:
            if (inst->value.power > inst->block->power)
            {
                inst->block->power = inst->value.power;
                inst->block->updated = true;
            }
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

Runmap* runmap_allocate(unsigned int size)
{
    return hashmap_allocate(size);
}

void runmap_free(Runmap* map)
{
    for (int i = 0; i < map->size; i++)
    for (Bucket* bucket = map->data + i; BUCKET_FILLED(bucket); bucket = bucket->next)
    for (RupInst* inst = bucket->value; inst != NULL;)
    {
        RupInst* temp = inst->next;
        free(inst);
        inst = temp;
    }
    hashmap_free(map);
}

void runmap_import(Runmap* map, Rup* rup)
{
    RupInst* next;
    for (RupInst* inst = rup->instructions; inst != NULL; inst = next)
    {
        next = inst->next;

        Bucket* bucket = hashmap_get(map, inst->location, true);
        if (bucket->value == NULL)
        {
            bucket->value = inst;
            inst->next = NULL;
            continue;
        }

        RupInst* found_inst = bucket->value;

        // First entry
        if (found_inst->command > inst->command)
        {
            inst->next = found_inst;
            bucket->value = inst;
            continue;
        }

        // Somewhere in the list
        for (;found_inst->next != NULL; found_inst = found_inst->next)
        {
            if (found_inst->next->command > inst->command)
            {
                inst->next = found_inst->next;
                found_inst->next = inst;
                continue;
            }
        }

        // Last entry
        found_inst->next = inst;
        inst->next = NULL;
    }

    rup->instructions = NULL;
}

bool runmap_block_power_changed(Runmap* map, Block* block)
{
    Bucket* bucket = hashmap_get(map, block->location, false);
    if (bucket == NULL)
        return false;

    RupInst* inst = bucket->value;
    return inst->command == RUP_POWER;

}

