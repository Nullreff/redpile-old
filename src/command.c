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

void command_ping(void)
{
    printf("PONG\n");
}

void command_status(void)
{
    world_stats_print(world_get_stats(world));
}

void command_set(Location location, Type* type, SetArgs args)
{
    Node* node = world_set_node(world, location, type);
    if (node != NULL)
    {
        FIELD_SET(node, 1, args.direction);
        FIELD_SET(node, 2, args.state);
    }
}

void command_setr(Location l1, Location l2, Type* type, SetArgs args)
{
    command_setrs(l1, l2, location_create(1, 1, 1), type, args);
}

void command_setrs(Location l1, Location l2, Location step, Type* type, SetArgs args)
{
    int x_start = l1.x > l2.x ? l2.x : l1.x;
    int x_end   = l1.x > l2.x ? l1.x : l2.x;
    int y_start = l1.y > l2.y ? l2.y : l1.y;
    int y_end   = l1.y > l2.y ? l1.y : l2.y;
    int z_start = l1.z > l2.z ? l2.z : l1.z;
    int z_end   = l1.z > l2.z ? l1.z : l2.z;

    for (int x = x_start; x <= x_end; x += step.x)
    for (int y = y_start; y <= y_end; y += step.y)
    for (int z = z_start; z <= z_end; z += step.z)
        command_set(location_create(x, y, z), type, args);
}

void command_get(Location location)
{
    Node* node = world_get_node(world, location);
    if (node == NULL)
        printf("(%d,%d,%d) EMPTY\n", location.x, location.y, location.z);
    else
        node_print(node);
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
    if (strcasecmp(string, "EMPTY") == 0)
    {
        *found_type = NULL;
        free(string);
        return true;
    }

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

bool direction_parse(char* string, Direction* found_dir)
{
    for (int i = 0; i < DIRECTIONS_COUNT; i++)
    {
        if (strcasecmp(string, Directions[i]) == 0)
        {
            *found_dir = i;
            free(string);
            return true;
        }
    }

    fprintf(stderr, "Unknown direction: '%s'\n", string);
    free(string);
    return false;
}
