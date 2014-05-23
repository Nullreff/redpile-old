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
    rup->touched = hashmap_allocate(15);
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
    hashmap_free(rup->touched);
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
    Bucket* bucket = hashmap_get(rup->touched, block->location, true);
    if (bucket->value != NULL)
    {
        RupInst* inst = (RupInst*)bucket->value;
        assert(inst->command == RUP_POWER);
        if (power > inst->value.power)
            inst->value.power = power;
    }
    else
    {
        RupInst* inst = rup_push(rup, RUP_POWER, block);
        inst->value.power = power;
        bucket->value = inst;
    }
}

void rup_cmd_state(Rup* rup, Block* block, unsigned int state)
{
    RupInst* inst = rup_push(rup, RUP_STATE, block);
    inst->value.state = state;
}

void rup_cmd_move(Rup* rup, Block* block, Block* source)
{
    RupInst* inst = rup_push(rup, RUP_MOVE, block);
    inst->value.source = source;
}

void rup_cmd_set(Rup* rup, Block* block, Material material)
{
    RupInst* inst = rup_push(rup, RUP_SET, block);
    inst->value.material = material;
}

RupInst* rup_get(Rup* rup, Block* block)
{
    Bucket* bucket = hashmap_get(rup->touched, block->location, false);
    return bucket != NULL ? (RupInst*)bucket->value : NULL;
}

unsigned int rup_get_power(Rup* rup, Block* block)
{
    RupInst* inst = rup_get(rup, block);
    assert(inst == NULL || inst->command == RUP_POWER);
    return inst != NULL && inst->command == RUP_POWER ? inst->value.power : block->power;
}

