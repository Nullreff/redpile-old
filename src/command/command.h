/* command/command.h - Command line instruction parser and dispatcher
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

#ifndef REDPILE_COMMAND_H
#define REDPILE_COMMAND_H

#include "../world.h"
#include "../redstone.h"

extern World* current_world;

void command_error(const char* message);
void command_ping(void);
void command_status(void);
void command_set(int x, int y, int z, char* material_str);
void command_get(int x, int y, int z);
void command_tick(int count, LogLevel log_level);
void command_messages(void);

#endif
