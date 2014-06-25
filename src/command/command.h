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
#include "../location.h"

extern World* current_world;

typedef struct {
    Direction direction;
    unsigned int state;
} SetArgs;

void command_ping(void);
void command_status(void);
void command_set(Location location, Type type, SetArgs args);
void command_get(Location location);
void command_tick(int count, LogLevel log_level);
void command_messages(void);

void command_error(const char* message);
void command_unknown(const char* command);

#endif
