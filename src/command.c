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
    unsigned int index;
    Field* field = type_find_field(node->data->type, name, &index);
    if (!field)
    {
        repl_print_error("The type '%s' doesn't have the field '%s'\n", node->data->type->name, name);
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

CommandArgs* command_args_allocate(unsigned int count)
{
    CommandArgs* args = malloc(sizeof(CommandArgs) + (sizeof(CommandArg) * count));
    args->count = count;
    args->index = 0;
    return args;
}

void command_args_free(CommandArgs* args)
{
    for (unsigned int i = 0; i < args->index; i++)
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

static void command_node_get_callback(Location location, Node* node, UNUSED void* args)
{
    if (NODE_IS_EMPTY(node) || node->data->type == NULL)
    {
        Type* type = type_data_get_default_type(world->type_data);
        repl_print("%d,%d,%d %s\n", location.x, location.y, location.z, type->name);
    }
    else
    {
        node_print(node);
    }
}

void command_node_get(Region* region)
{
    world_get_region(world, region, command_node_get_callback, NULL);
    free(region);
}

struct command_node_set_args {
    Type* type;
    CommandArgs* fields;
};

static void command_node_set_callback(UNUSED Location location, Node* node, void* args)
{
    Type* type = ((struct command_node_set_args*)args)->type;
    node->data->type = type;

    CommandArgs* fields = ((struct command_node_set_args*)args)->fields;
    for (unsigned int i = 0; i < fields->index; i++)
        node_field_set(node, fields->data[i].name, fields->data[i].value);
}

void command_node_set(Region* region, char* type_name, CommandArgs* fields)
{
    Type* type;
    if (type_parse(type_name, &type))
    {
        struct command_node_set_args args = {type, fields};
        world_set_region(world, region, command_node_set_callback, &args);
    }

    free(region);
    command_args_free(fields);
}

static void command_field_get_callback(Location location, Node* node, void* args)
{
    char* name = (char*)args;
    Field* field;
    unsigned int index;
    if (node->data->type && node->data->fields && (field = type_find_field(node->data->type, name, &index)))
        node_print_field_value(node, field->type, node->data->fields->data[index]);
    else
        repl_print("%d,%d,%d nil\n", location.x, location.y, location.z);
}

void command_field_get(Region* region, char* name)
{
    world_get_region(world, region, command_field_get_callback, name);
    free(region);
    free(name);
}

struct command_field_set_args {
    char* name;
    char* value;
};

static void command_field_set_callback(UNUSED Location location, Node* node, void* args)
{
    char* name = ((struct command_field_set_args*)args)->name;
    char* value = ((struct command_field_set_args*)args)->value;

    if (!NODE_IS_EMPTY(node) && node->data->type)
    {
        node_field_set(node, name, value);
    }
    else
    {
        Type* type = type_data_get_default_type(world->type_data);
        repl_print_error("The type '%s' doesn't have the field '%s'\n", type->name, name);
    }
}

void command_field_set(Region* region, char* name, char* value)
{
    struct command_field_set_args args = {name, value};
    world_get_region(world, region, command_field_set_callback, &args);

    free(region);
    free(name);
    free(value);
}

void command_delete(Region* region)
{
    world_delete_region(world, region);
    free(region);
}

struct command_plot_args {
    char* field;
    int start;
    int end;
};

static void command_plot_field(Node* node, char* name)
{
    unsigned int index;
    Field* field = NULL;
    if (node->data != NULL)
        field = type_find_field(node->data->type, name, &index);

    if (field)
    {
        switch (field->type)
        {
            case FIELD_INTEGER: {
                int value = FIELD_GET(node, index, integer);
                printf("%3d ", value);
                break;
            }

            case FIELD_DIRECTION: {
                Direction dir = FIELD_GET(node, index, integer);
                printf(" %c  ", direction_to_letter(dir));
                break;
            }

            case FIELD_STRING: {
                char* str = FIELD_GET(node, index, string);
                if (str && str[0] != '\0')
                    printf(" @  ");
                else
                    printf(" *  ");
                break;
            }
        }
    }
    else
    {
        printf("    ");
    }
}

static void command_plot_z_callback(Location location, Node* node, void* args)
{
    struct command_plot_args* data = (struct command_plot_args*)args;

    if (location.z == data->start)
        printf("|");

    command_plot_field(node, data->field);

    if (location.z == data->end)
        printf("\n");
}

static void command_plot_y_callback(Location location, Node* node, void* args)
{
    struct command_plot_args* data = (struct command_plot_args*)args;

    if (location.y == data->start)
        printf("|");

    command_plot_field(node, data->field);

    if (location.y == data->end)
        printf("\n");
}

void command_plot(Region* region, char* field)
{
    printf("+");
    if (region->x.start == region->x.end)
    {
        for (int i = region->z.start; i <= region->z.end; i++)
            printf("----");
        printf("Z\n");
        struct command_plot_args args = {field, region->z.start, region->z.end};
        world_get_region(world, region, command_plot_z_callback, &args);
        printf("Y\n");
    }
    else if (region->y.start == region->y.end)
    {
        for (int i = region->z.start; i <= region->z.end; i++)
            printf("----");
        printf("Z\n");
        struct command_plot_args args = {field, region->z.start, region->z.end};
        world_get_region(world, region, command_plot_z_callback, &args);
        printf("X\n");
    }
    else if (region->z.start == region->z.end)
    {
        for (int i = region->y.start; i <= region->y.end; i++)
            printf("----");
        printf("Y\n");
        struct command_plot_args args = {field, region->y.start, region->y.end};
        world_get_region(world, region, command_plot_y_callback, &args);
        printf("X\n");
    }
    else
    {
        repl_print_error("The region provided must be flat");
    }

    free(region);
    free(field);
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

