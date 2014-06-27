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
#include <stdlib.h>
#include <stdio.h>

World* current_world;

void command_ping(void)
{
    printf("PONG\n");
}

void command_status(void)
{
    world_stats_print(world_get_stats(current_world));
}

void command_set(Location location, Type type, SetArgs args)
{
    Node* node = world_set_node(current_world, location, type);
    if (node != NULL)
    {
        FIELD_SET(node, 1, args.direction);
        FIELD_SET(node, 2, args.state);
    }
}

void command_setr(Location l1, Location l2, Type type, SetArgs args)
{
    int x_start = l1.x > l2.x ? l2.x : l1.x;
    int x_end   = l1.x > l2.x ? l1.x : l2.x;
    int y_start = l1.y > l2.y ? l2.y : l1.y;
    int y_end   = l1.y > l2.y ? l1.y : l2.y;
    int z_start = l1.z > l2.z ? l2.z : l1.z;
    int z_end   = l1.z > l2.z ? l1.z : l2.z;

    for (int x = x_start; x <= x_end; x++)
    for (int y = y_start; y <= y_end; y++)
    for (int z = z_start; z <= z_end; z++)
        command_set(location_create(x, y, z), type, args);
}

void command_setrs(Location l1, Location l2, Location step, Type type, SetArgs args)
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
    Node* node = world_get_node(current_world, location);
    if (node == NULL)
        printf("(%d,%d,%d) EMPTY\n", location.x, location.y, location.z);
    else
        node_print(node);
}

void command_tick(int count, LogLevel log_level)
{
    if (count > 0)
        logic_run_tick(current_world, count, log_level);
}

void command_messages(void)
{
    world_print_messages(current_world);
}

void command_error(const char* message)
{
    fprintf(stderr, "%s\n", message);
}

