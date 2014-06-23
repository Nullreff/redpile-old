/* instruction.h - Command line instruction parser and dispatcher
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

#ifndef REDPILE_INSTRUCTION_H
#define REDPILE_INSTRUCTION_H

#include "world.h"
#include "rup.h"

#define COMMANDS_COUNT 7
extern char* Commands[COMMANDS_COUNT];
typedef enum {
    PING,
    STATUS,
    SET,
    GET,
    TICK,
    VTICK,
    MESSAGES
} Command;

typedef struct {
    Command cmd;
    int values[6];
} Instruction;

bool instruction_parse(char* instruction, Instruction* result);
void instruction_run(World* world, Instruction* inst, void (*rup_inst_run_callback)(RupNode*));

#endif

