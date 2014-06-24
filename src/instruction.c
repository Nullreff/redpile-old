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
#include "redstone.h"
#include <ctype.h>

#define PARSE_STRING(NAME) do {\
    char* str_ ## NAME = strsep(&parts, " ");\
    if (str_ ## NAME == NULL)\
        goto success;\
    if(!NAME ## _parse(str_ ## NAME, &NAME))\
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
    "PING",
    "STATUS",
    "SET",
    "GET",
    "TICK",
    "VTICK",
    "STICK",
    "MESSAGES"
};

static bool command_parse(char* command, Command* result)
{
    for (int i = 0; i < COMMANDS_COUNT; i++)
    {
        if (strcasecmp(command, Commands[i]) == 0)
        {
            *result = (Command)i;
            return true;
        }
    }

    return false;
}

static bool material_parse(char* material, Material* result)
{
    for (int i = 0; i < MATERIALS_COUNT; i++)
    {
        if (strcasecmp(material, Materials[i]) == 0)
        {
            *result = (Material)i;
            return true;
        }
    }

    return false;
}

static bool direction_parse(char* direction, Direction* result)
{
    for (int i = 0; i < DIRECTIONS_COUNT; i++)
    {
        if (strcasecmp(direction, Directions[i]) == 0)
        {
            *result = (Direction)i;
            return true;
        }
    }

    return false;
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
    if (command == STATUS || command == PING || command == MESSAGES)
        goto success;

    if (command == TICK || command == VTICK || command == STICK)
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

void instruction_run(World* world, Instruction* inst)
{
    Location location;
    Node* node;

    switch (inst->cmd)
    {
        case PING:
            printf("PONG\n");
            break;

        case STATUS:
            world_stats_print(world_get_stats(world));
            break;

        case SET:
            location = location_from_values(inst->values);
            node = world_set_node(world, location, inst->values[3]);
            if (node != NULL)
            {
                FIELD_SET(node, 1, inst->values[4]);
                FIELD_SET(node, 2, inst->values[5]);
            }
            return;

        case GET:
            location = location_from_values(inst->values);
            node = world_get_node(world, location);
            if (node == NULL)
                printf("(%d,%d,%d) EMPTY", location.x, location.y, location.z);
            else
                node_print(node);
            break;

        case TICK:
        case VTICK:
        case STICK:
            if (inst->values[0] > 0)
                redstone_tick(world, inst->values[0],
                    inst->cmd == VTICK ? VERBOSE :
                    inst->cmd == STICK ? SILENT :
                    NORMAL);
            break;

        case MESSAGES:
            world_print_messages(world);
            break;
    }
}

