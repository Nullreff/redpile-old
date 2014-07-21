/* command.c - Command line instruction dispatcher
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

#include "command.h"
#include "common.h"
#include "redpile.h"

static bool direction_parse(char* string, Direction* found_dir)
{
    for (int i = 0; i < DIRECTIONS_COUNT; i++)
    {
        if (strcasecmp(string, Directions[i]) == 0)
        {
            *found_dir = i;
            return true;
        }
    }

    return false;
}

static bool integer_parse(char* string, int* found_int)
{
    char* parse_error = NULL;
    int value = strtol(string, &parse_error, 10);

    if (!*parse_error)
    {
        *found_int = value;
        return true;
    }
    else
    {
        return false;
    }
}

void run_command_set(Location location, Type* type, CommandArgs* args)
{
    Node* node = world_set_node(world, location, type);
    assert(node != NULL);

    for (int i = 0; i < args->index; i++)
    {
        CommandArg* arg = args->data + i;

        int index;
        FieldType field_type;
        if (!type_find_field(node->type, arg->name, &index, &field_type))
        {
            fprintf(stderr, "The type '%s' doesn't have the field '%s'\n", node->type->name, arg->name);
            return;
        }

        switch (field_type)
        {
            case FIELD_INT: {
                int found_int;
                if (integer_parse(arg->value, &found_int))
                {
                    FIELD_SET(node, index, found_int);
                }
                else
                {
                    fprintf(stderr, "'%s' is not an integer\n", arg->value);
                    return;
                }
            }
            break;

            case FIELD_DIRECTION: {
                Direction found_dir;
                if (direction_parse(arg->value, &found_dir))
                {
                    FIELD_SET(node, index, found_dir);
                }
                else
                {
                    fprintf(stderr, "'%s' is not a direction\n", arg->value);
                    return;
                }
            }
            break;
        }
    }
}

CommandArgs* command_args_allocate(unsigned int count)
{
    CommandArgs* args = malloc(sizeof(CommandArgs) + (sizeof(CommandArg) * count));
    args->count = count;
    args->index = 0;
    return args;
}

void command_args_free(CommandArgs* args)
{
    for (int i = 0; i < args->index; i++)
    {
        free(args->data[i].name);
        free(args->data[i].value);
    }
    free(args);
}

void command_args_append(CommandArgs* args, char* name, char* value)
{
    args->data[args->index++] = (CommandArg){name, value};
}

void command_ping(void)
{
    printf("PONG\n");
}

void command_status(void)
{
    world_stats_print(world_get_stats(world));
}

void command_set(Location location, Type* type, CommandArgs* args)
{
    run_command_set(location, type, args);
    command_args_free(args);
}

void command_setr(Location l1, Location l2, Type* type, CommandArgs* args)
{
    command_setrs(l1, l2, location_create(1, 1, 1), type, args);
}

#define PARSE_ERROR_IF(CONDITION, ...) if (CONDITION) { fprintf(stderr, __VA_ARGS__); goto end; }
void command_setrs(Location l1, Location l2, Location step, Type* type, CommandArgs* args)
{
    PARSE_ERROR_IF(step.x <= 0, "x_step must be greater than zero\n");
    PARSE_ERROR_IF(step.y <= 0, "y_step must be greater than zero\n");
    PARSE_ERROR_IF(step.z <= 0, "z_step must be greater than zero\n");

    int x_start = l1.x > l2.x ? l2.x : l1.x;
    int x_end   = l1.x > l2.x ? l1.x : l2.x;
    int y_start = l1.y > l2.y ? l2.y : l1.y;
    int y_end   = l1.y > l2.y ? l1.y : l2.y;
    int z_start = l1.z > l2.z ? l2.z : l1.z;
    int z_end   = l1.z > l2.z ? l1.z : l2.z;

    for (int x = x_start; x <= x_end; x += step.x)
    for (int y = y_start; y <= y_end; y += step.y)
    for (int z = z_start; z <= z_end; z += step.z)
        run_command_set(location_create(x, y, z), type, args);

end:
    command_args_free(args);
}

void command_delete(Location location)
{
    world_remove_node(world, location);
}

void command_get(Location location)
{
    Node* node = world_get_node(world, location);
    if (node == NULL)
    {
        Type* type = type_data_get_default_type(world->type_data);
        printf("(%d,%d,%d) %s\n", location.x, location.y, location.z, type->name);
    }
    else
    {
        node_print(node);
    }
}

void command_tick(int count, LogLevel log_level)
{
    if (count > 0)
        tick_run(state, world, count, log_level);
}

void command_messages(void)
{
    world_print_messages(world);
}

void command_error(const char* message)
{
    fprintf(stderr, "%s\n", message);
}

bool type_parse(char* string, Type** found_type)
{
    Type* type = type_data_find_type(world->type_data, string);
    if (type != NULL)
    {
        *found_type = type;
        free(string);
        return true;
    }

    fprintf(stderr, "Unknown type: '%s'\n", string);
    free(string);
    return false;
}

