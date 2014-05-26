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
#include "world.h"

#define RUP_CMD_COUNT 4
typedef enum {
    RUP_POWER = 0,
    RUP_STATE = 1,
    RUP_SWAP  = 2,
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

typedef struct {;
    RupInst* instructions[RUP_CMD_COUNT];
} Runmap;

Rup* rup_allocate(void);
void rup_free(Rup* rup);
void rup_cmd_power(Rup* rup, Block* block, unsigned int power);
void rup_cmd_state(Rup* rup, Block* block, unsigned int state);
void rup_cmd_swap(Rup* rup, Block* block, Block* target);
RupInst rup_inst_create(RupCmd cmd, Block* block);
void rup_inst_run(World* world, RupInst* inst);
void rup_inst_print(RupInst* inst);

Runmap* runmap_allocate(void);
void runmap_free(Runmap* runmap);
void runmap_import(Runmap* runmap, Rup* rup);

#endif
