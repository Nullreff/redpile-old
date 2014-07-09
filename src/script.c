/* script.c - Scripting engine built on lua
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

#include "script.h"
#include "type.h"

#define LUA_ERROR_IF(CONDITION,MESSAGE) if (CONDITION) { lua_pushstring(state, MESSAGE); lua_error(state); }

static int script_define_behavior(lua_State* state)
{
    LUA_ERROR_IF(!lua_isstring(state, -1), "You must pass a behavior name\n");
    const char* name = lua_tostring(state, -1);
    printf("Defining behavior: %s\n", name);
    return 0;
}

static int script_define_type(lua_State* state)
{
    LUA_ERROR_IF(!lua_isstring(state, -1), "You must pass a type name\n");
    const char* name = lua_tostring(state, -1);
    printf("Defining type: %s\n", name);
    return 0;
}

ScriptState* script_state_allocate(void)
{
    lua_State* state = luaL_newstate();

    static const luaL_Reg lualibs[] =
    {
        { "base", luaopen_base },
        { NULL, NULL}
    };
    const luaL_Reg *lib = lualibs;
    for(; lib->func != NULL; lib++)
    {
        lib->func(state);
        lua_settop(state, 0);
    }

    lua_pushcfunction(state, script_define_behavior);
    lua_setglobal(state, "define_behavior");
    lua_pushcfunction(state, script_define_type);
    lua_setglobal(state, "define_type");

    return state;
}

void script_state_free(ScriptState* state)
{
    lua_close(state);
}

TypeList* script_state_load_types(ScriptState* state, const char* config_file)
{
    int error = luaL_dofile(state, config_file);
    if (error)
    {
        printf("%s\n", lua_tostring(state, -1));
        return NULL;
    }

    return type_list_allocate(0);
}

