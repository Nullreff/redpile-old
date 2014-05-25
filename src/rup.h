/* rup.h - Redstone update programming language
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

#ifndef REDPILE_RUP_H
#define REDPILE_RUP_H

#include "block.h"
#include "hashmap.h"

typedef enum {
    RUP_POWER = 0,
    RUP_STATE = 1,
    RUP_MOVE  = 2,
    RUP_SET   = 3
} RupCmd;

typedef struct RupInst {
    RupCmd command;
    Location location;
    Block* block;
    union {
        unsigned int power;
        unsigned int state;
        Material material;
        Block* target;
    } value;
    struct RupInst* next;
} RupInst;

typedef struct {
    RupInst* instructions;
} Rup;

typedef struct {
    Rup** rups;
    unsigned int size;
} RupList;

typedef Hashmap Runmap;

Rup* rup_allocate(void);
void rup_free(Rup* rup);
void rup_cmd_power(Rup* rup, Block* block, unsigned int power);
void rup_cmd_state(Rup* rup, Block* block, unsigned int state);
void rup_cmd_move(Rup* rup, Block* block, Block* target);
void rup_cmd_set(Rup* rup, Block* block, Material material);
void rup_run(RupInst* inst);

RupList* rup_list_allocate(unsigned int size);
void rup_list_free(RupList* list);

Runmap* runmap_allocate(unsigned int size);
void runmap_free(Runmap* map);
void runmap_import(Runmap* map, Rup* rup);
bool runmap_block_power_changed(Runmap* map, Block* block);

#endif
