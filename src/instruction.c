/* instruction.c - Command line instruction parser and dispatcher
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
#include <stdio.h>
#include <string.h>
#include "instruction.h"
#include "redpile.h"
#include "block.h"
#include "world.h"
#include "redstone.h"

#define PARSE_NUMBER(NAME)\
    char* str_ ## NAME = strtok(NULL, " ");\
    if (str_ ## NAME == NULL)\
        goto error;\
    NAME = strtol(str_ ## NAME , &parse_error, 10);\
    if (*parse_error)\
        goto error;

int command_parse(char* command, Command* result)
{
    if (strcmp(command, "SET") == 0)
        *result = CMD_SET;
    else if (strcmp(command, "GET") == 0)
        *result = CMD_GET;
    else if (strcmp(command, "TICK") == 0)
        *result = CMD_TICK;
    else if (strcmp(command, "STATUS") == 0)
        *result = CMD_STATUS;
    else
        return -1;

    return 0;
}

int instruction_parse(char* instruction, Instruction* result)
{
    char* parts = strdup(instruction);
    char* parts_ptr = parts;
    CHECK_OOM(parts);

    Command command;
    Material material;
    Coord x, y, z;
    x = y = z = 0;

    char* str_command = strtok(parts, " ");
    if (str_command == NULL || command_parse(str_command, &command) == -1)
    {
        goto error;
    }

    if (command == CMD_TICK || command == CMD_STATUS)
    {
        goto success;
    }

    char* parse_error;
    PARSE_NUMBER(x)
    PARSE_NUMBER(y)
    PARSE_NUMBER(z)

    if (command == CMD_GET)
    {
        goto success;
    }

    char* str_material = strtok(NULL, " ");
    if (str_material == NULL || material_parse(str_material, &material) == -1)
    {
        goto error;
    }

success:
    *result = (Instruction){command, (Location){x, y, z}, material};
    free(parts_ptr);
    return 0;

error:
    free(parts_ptr);
    return -1;
}

void instruction_run(World* world, Instruction* inst, void (*block_modified_callback)(Block*))
{
    Block new_block;
    Block* block;

    switch (inst->cmd)
    {
        case CMD_SET:
            new_block = block_create((Material)inst->value, inst->target);
            block = world_set_block(world, &new_block);
            return;

        case CMD_GET:
            block = world_get_block(world, inst->target);
            if (block == NULL)
            {
                new_block = block_create(M_EMPTY, inst->target);
                block_modified_callback(&new_block);
            }
            else
            {
                block_modified_callback(block);
            }
            break;

        case CMD_TICK:
            redstone_tick(world, block_modified_callback);
            break;

        case CMD_STATUS:
            world_print_status(world);
            break;
    }
}

