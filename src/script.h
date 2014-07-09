/* script.h - Scripting engine built on lua
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

#ifndef REDPILE_SCRIPT_H
#define REDPILE_SCRIPT_H

#include "type.h"
#include "world.h"
#include "queue.h"
#include "node.h"
#include "message.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

typedef lua_State ScriptState;

typedef struct {
    World* world;
    Node* node;
    Messages* input;
    Queue* messages;
    Queue* sets;
} ScriptData;

typedef enum {
    COMPLETE,
    INCOMPLETE,
    ERROR
} Result;

ScriptState* script_state_allocate(void);
void script_state_free(ScriptState* state);
TypeData* script_state_load_config(ScriptState* state, const char* config_file);
Result script_state_run_behavior(ScriptState* state, Behavior* behavior, ScriptData* data);

#endif
