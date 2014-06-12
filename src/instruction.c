/* instruction.c - Command line instruction parser and dispatcher
 *
 * Copyright (C) 2014 Ryan Mendivil <ryan@nullreff.net>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "instruction.h"
#include "redpile.h"
#include "block.h"
#include "world.h"
#include "redstone.h"
#include <ctype.h>

#define PARSE_STRING(NAME) do {\
    char* str_ ## NAME = strsep(&parts, " ");\
    if (str_ ## NAME == NULL)\
        goto success;\
    if(NAME ## _parse(str_ ## NAME, &NAME) == -1)\
        goto error;\
} while (0)

#define PARSE_NUMBER(NAME) do {\
    char* str_ ## NAME = strsep(&parts, " ");\
    if (str_ ## NAME == NULL)\
        goto success;\
    NAME = strtol(str_ ## NAME , &parse_error, 10);\
    if (*parse_error)\
        goto error;\
} while (0)

char* Commands[COMMANDS_COUNT] = {
    "SET",
    "GET",
    "TICK",
    "STATUS",
    "PING"
};

static int command_parse(char* command, Command* result)
{
    for (int i = 0; i < COMMANDS_COUNT; i++)
    {
        if (strcasecmp(command, Commands[i]) == 0)
        {
            *result = (Command)i;
            return 0;
        }
    }

    return -1;
}

bool instruction_parse(char* instruction, Instruction* result)
{
    char* parts = strdup(instruction);
    CHECK_OOM(parts);

    char* parts_ptr = parts;
    char* parse_error;

    Command command;
    Coord x = 0;
    Coord y = 0;
    Coord z = 0;
    Material material = MATERIAL_DEFAULT;
    Direction direction = DIRECTION_DEFAULT;
    int state = 0;

    PARSE_STRING(command);
    if (command == STATUS || command == PING)
        goto success;

    if (command == TICK)
    {
        if (parts != NULL)
            PARSE_NUMBER(x);
        else
            x = 1;
        goto success;
    }

    PARSE_NUMBER(x);
    PARSE_NUMBER(y);
    PARSE_NUMBER(z);
    if (command == GET)
        goto success;

    PARSE_STRING(material);
    PARSE_STRING(direction);
    PARSE_NUMBER(state);

    if (state < 0 || state > 3)
        goto error;

success:
    *result = (Instruction){command, {x, y, z, material, direction, state}};
    free(parts_ptr);
    return true;

error:
    free(parts_ptr);
    return false;
}

void instruction_run(World* world, Instruction* inst, void (*rup_inst_run_callback)(RupNode*))
{
    Block new_block;
    Block* block;

    switch (inst->cmd)
    {
        case SET:
            new_block = block_from_values(inst->values + 3);
            world_set_block(world, location_from_values(inst->values), &new_block, false);
            return;

        case GET:
            block = world_get_block(world, location_from_values(inst->values));
            if (block == NULL)
            {
                new_block = block_empty();
                block_print(&new_block);
            }
            else
            {
                block_print(block);
            }
            break;

        case TICK:
            if (inst->values[0] > 0)
                redstone_tick(world, rup_inst_run_callback, inst->values[0]);
            break;

        case STATUS:
            world_stats_print(world_get_stats(world));
            break;

        case PING:
            printf("PONG\n");
            break;
    }
}

