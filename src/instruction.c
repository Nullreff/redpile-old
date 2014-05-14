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

#include "instruction.h"
#include "redpile.h"
#include "block.h"
#include "world.h"
#include "redstone.h"
#include <ctype.h>

#define PARSE_NUMBER(NAME)\
    char* str_ ## NAME = strtok(NULL, " ");\
    if (str_ ## NAME == NULL)\
        goto error;\
    NAME = strtol(str_ ## NAME , &parse_error, 10);\
    if (*parse_error)\
        goto error;

char* Commands[COMMANDS_COUNT] = {
    "SET",
    "GET",
    "TICK",
    "STATUS",
    "PING"
};

int command_parse(char* command, Command* result)
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

int instruction_parse(char* instruction, Instruction* result)
{
    char* parts = strdup(instruction);
    char* parts_ptr = parts;
    CHECK_OOM(parts);

    Command command;
    Coord x = 0;
    Coord y = 0;
    Coord z = 0;
    Material material = EMPTY;
    Direction direction = NORTH;

    char* str_command = strtok(parts, " ");
    if (str_command == NULL || command_parse(str_command, &command) == -1)
        goto error;

    if (command == TICK || command == STATUS || command == PING)
        goto success;

    char* parse_error;
    PARSE_NUMBER(x)
    PARSE_NUMBER(y)
    PARSE_NUMBER(z)

    if (command == GET)
        goto success;

    char* str_material = strtok(NULL, " ");
    if (str_material == NULL || material_parse(str_material, &material) == -1)
        goto error;

    if (!HAS_DIRECTION(material))
        goto success;

    char* str_direction = strtok(NULL, " ");
    if (str_direction == NULL || direction_parse(str_direction, &direction) == -1)
        goto error;

success:
    *result = (Instruction){command, {x, y, z, material, direction}};
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
        case SET:
            new_block = block_from_values(inst->values);
            world_set_block(world, &new_block);
            return;

        case GET:
            block = world_get_block(world, location_from_values(inst->values));
            if (block == NULL)
            {
                new_block = block_create(location_from_values(inst->values), EMPTY, NORTH);
                block_print(&new_block);
            }
            else
            {
                block_print(block);
            }
            break;

        case TICK:
            redstone_tick(world, block_modified_callback);
            break;

        case STATUS:
            world_stats_print(world_get_stats(world));
            break;

        case PING:
            printf("PONG\n");
            break;
    }
}

