/* command.c - Command line instruction dispatcher
 *
 * Copyright (C) 2014 Ryan Mendivil <ryan@nullreff.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redpile nor the names of its contributors may be
 *     used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "command.h"
#include "common.h"
#include "redpile.h"
#include "repl.h"

#define PARSE_ERROR_IF(CONDITION, ...) if (CONDITION) { repl_print_error(__VA_ARGS__); goto end; }
#define FOR_REGION(R)\
    int x_start = (R)->x.start > (R)->x.end ? (R)->x.end : (R)->x.start;\
    int x_end   = (R)->x.start > (R)->x.end ? (R)->x.start : (R)->x.end;\
    int y_start = (R)->y.start > (R)->y.end ? (R)->y.end : (R)->y.start;\
    int y_end   = (R)->y.start > (R)->y.end ? (R)->y.start : (R)->y.end;\
    int z_start = (R)->z.start > (R)->z.end ? (R)->z.end : (R)->z.start;\
    int z_end   = (R)->z.start > (R)->z.end ? (R)->z.start : (R)->z.end;\
    int x_step  = abs((R)->x.step);\
    int y_step  = abs((R)->y.step);\
    int z_step  = abs((R)->z.step);\
    for (int x = x_start; x <= x_end; x += x_step)\
    for (int y = y_start; y <= y_end; y += y_step)\
    for (int z = z_start; z <= z_end; z += z_step)

static bool direction_parse(const char* string, Direction* found_dir)
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

static bool integer_parse(const char* string, int* found_int)
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

static bool type_parse(char* string, Type** type)
{
    *type = type_data_find_type(world->type_data, string);
    if (*type != NULL)
    {
        free(string);
        return true;
    }

    repl_print_error("Unknown type '%s'\n", string);
    free(string);
    return false;
}

static void node_field_set(Node* node, char* name, char* value)
{
    int index;
    Field* field = type_find_field(node->type, name, &index);
    if (!field)
    {
        repl_print_error("The type '%s' doesn't have the field '%s'\n", node->type->name, name);
        return;
    }

    switch (field->type)
    {
        case FIELD_INTEGER: {
            int result;
            if (integer_parse(value, &result))
            {
                FIELD_SET(node, index, integer, result);
            }
            else
            {
                repl_print_error("'%s' is not an integer\n", value);
                return;
            }
        } break;

        case FIELD_DIRECTION: {
            Direction result;
            if (direction_parse(value, &result))
            {
                FIELD_SET(node, index, direction, result);
            }
            else
            {
                repl_print_error("'%s' is not a direction\n", value);
                return;
            }
        } break;

        case FIELD_STRING:
            FIELD_SET(node, index, string, strdup(value));
            break;
    }
}

static void run_command_node_set(Location location, Type* type, CommandArgs* args)
{
    Node* node = world_set_node(world, location, type);
    assert(node != NULL);

    for (int i = 0; i < args->index; i++)
        node_field_set(node, args->data[i].name, args->data[i].value);
}

Range range_create(int start, int end, unsigned int step)
{
    return (Range){start, end, step};
}

Region* region_allocate(Range x, Range y, Range z)
{
    Region* region = malloc(sizeof(Region));
    region->x = x;
    region->y = y;
    region->z = z;
    return region;
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
    repl_print("PONG\n");
}

void command_status(void)
{
    world_stats_print(world_get_stats(world));
}

void command_node_get(Region* region)
{
    FOR_REGION(region)
    {
        Location location = location_create(x, y, z);
        Node* node = world_get_node(world, location);
        if (node == NULL)
        {
            Type* type = type_data_get_default_type(world->type_data);
            repl_print("(%d,%d,%d) %s\n", location.x, location.y, location.z, type->name);
        }
        else
        {
            node_print(node);
        }
    }
    free(region);
}

void command_node_set(Region* region, char* type_name, CommandArgs* args)
{
    Type* type;
    if (type_parse(type_name, &type))
    {
        FOR_REGION(region)
            run_command_node_set(location_create(x, y, z), type, args);
    }
    free(region);
    command_args_free(args);
}

void command_field_get(Region* region, char* name)
{
    FOR_REGION(region)
    {
        Location location = location_create(x, y, z);
        Node* node = world_get_node(world, location);
        if (!node)
        {
            repl_print("(%d,%d,%d) nil\n", location.x, location.y, location.z);
            return;
        }

        int index;
        Field* field = type_find_field(node->type, name, &index);
        if (!field)
        {
            repl_print("(%d,%d,%d) nil\n", location.x, location.y, location.z);
            return;
        }

        node_print_field_value(node, field->type, node->fields.data[index]);
    }
    free(region);
    free(name);
}

void command_field_set(Region* region, char* name, char* value)
{
    FOR_REGION(region)
    {
        Location location = location_create(x, y, z);
        Node* node = world_get_node(world, location);
        if (!node)
        {
            Type* type = type_data_get_default_type(world->type_data);
            repl_print_error("The type '%s' doesn't have the field '%s'\n", type->name, name);
            return;
        }

        node_field_set(node, name, value);
    }
    free(region);
    free(name);
    free(value);
}

void command_delete(Region* region)
{
    FOR_REGION(region)
        world_remove_node(world, location_create(x, y, z));
    free(region);
}

void command_tick(int count, LogLevel log_level)
{
    if (count > 0)
        tick_run(state, world, count, log_level);
}

void command_message(void)
{
    world_print_messages(world);
}

void command_type_list(void)
{
    world_print_types(world);
}

void command_type_show(char* name)
{
    world_print_type(world, name);
    free(name);
}

void command_error(const char* message)
{
    repl_print_error("%s\n", message);
}

