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

void rup_add(Rup* rup, RupCmd cmd, Block* block, unsigned int value)
{
    Bucket* bucket = hashmap_get(rup->touched, block->location, true);
    if (bucket->value == NULL)
    {
        RupInst* inst = malloc(sizeof(RupInst));
        *inst = (RupInst){cmd, block->location, block, value, rup->instructions};
        rup->instructions = inst;
        bucket->value = inst;
    }
    else
    {
        RupInst* inst = (RupInst*)bucket->value;
        if (value > inst->value)
            inst->value = value;
    }
}

RupInst* rup_get(Rup* rup, Block* block)
{
    Bucket* bucket = hashmap_get(rup->touched, block->location, false);
    return bucket != NULL ? (RupInst*)bucket->value : NULL;
}

unsigned int rup_get_power(Rup* rup, Block* block)
{
    RupInst* inst = rup_get(rup, block);
    return inst != NULL ? inst->value : block->power;
}

