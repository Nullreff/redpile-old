/* command/command.c - Command line instruction parser and dispatcher
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

void command_ping(void)
{
    printf("PONG\n");
}

void command_status(void)
{
    world_stats_print(world_get_stats(current_world));
}

void command_set(int x, int y, int z, char* material_str)
{
    Material material = 0;
    if (!material_parse(material_str, &material))
    {
        fprintf(stderr, "Invalid material: %s\n", material_str);
        return;
    }

    world_set_node(current_world, location_create(x, y, z), material);
}

void command_get(int x, int y, int z)
{
    Location location = location_create(x, y, z);
    Node* node = world_get_node(current_world, location);
    if (node == NULL)
        printf("(%d,%d,%d) EMPTY\n", location.x, location.y, location.z);
    else
        node_print(node);
}

void command_tick(int count, LogLevel log_level)
{
    if (count > 0)
        redstone_tick(current_world, count, log_level);
}

void command_messages(void)
{
    world_print_messages(current_world);
}

void command_error(const char* message)
{
    fprintf(stderr, "%s\n", message);
}

void command_unknown(const char* command)
{
    fprintf(stderr, "Unknown command '%s'\n", command);
}

