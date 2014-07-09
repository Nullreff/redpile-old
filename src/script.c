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
#include "common.h"

#define LUA_ERROR_IF(CONDITION,MESSAGE) if (CONDITION) { lua_pushstring(state, MESSAGE); lua_error(state); }
#define IS_UINT(NUM) ((NUM - ((double)(int)NUM) == 0) && (NUM >= 0))

TypeData* type_data;

static int script_define_behavior(lua_State* state)
{
    LUA_ERROR_IF(!lua_isstring(state, 1), "You must pass a behavior name");
    LUA_ERROR_IF(!lua_isnumber(state, 2), "You must pass a behavior mask");
    double raw_mask = lua_tonumber(state, 2);
    LUA_ERROR_IF(!IS_UINT(raw_mask), "Behavior mask must be a positive integer");
    LUA_ERROR_IF(!lua_isfunction(state, 3), "You must pass a behavior function");

    int function_ref = luaL_ref(state, LUA_REGISTRYINDEX);
    char* name = strdup(lua_tostring(state, 1));
    unsigned int mask = raw_mask;

    type_data_append_behavior(type_data, name, mask, function_ref);
    return 0;
}

static int script_define_type(lua_State* state)
{
    LUA_ERROR_IF(!lua_isstring(state, 1), "You must pass a type name");
    LUA_ERROR_IF(!lua_isnumber(state, 2), "You must pass the number of fields");
    double raw_field_count = lua_tonumber(state, 2);
    LUA_ERROR_IF(!IS_UINT(raw_field_count), "Number of fields must be a positive integer");

    char* name = strdup(lua_tostring(state, 1));
    unsigned int field_count = raw_field_count;
    unsigned int behavior_count = lua_gettop(state) - 2;

    Type* type = type_data_append_type(type_data, name, field_count, behavior_count);
    for (int i = 0; i < behavior_count; i++)
    {
        const char* name = luaL_checkstring(state, 3 + i);
        Behavior* behavior = type_data_find_behavior(type_data, name);
        ERROR_IF(behavior == NULL, "Could not find behavior");
        type->behaviors[i] = behavior;
    }

    return 0;
}

ScriptState* script_state_allocate(void)
{
    lua_State* state = luaL_newstate();

    static const luaL_Reg lualibs[] =
    {
        { "base", luaopen_base },
        { "bit32", luaopen_bit32 },
        { NULL, NULL}
    };
    const luaL_Reg *lib = lualibs;
    for(; lib->func != NULL; lib++)
    {
        lib->func(state);
        lua_settop(state, 0);
    }

    lua_pushnumber(state, MESSAGE_POWER);
    lua_setglobal(state, "MESSAGE_POWER");
    lua_pushnumber(state, MESSAGE_PUSH);
    lua_setglobal(state, "MESSAGE_PUSH");
    lua_pushnumber(state, MESSAGE_PULL);
    lua_setglobal(state, "MESSAGE_PULL");
    lua_pushnumber(state, MESSAGE_REMOVE);
    lua_setglobal(state, "MESSAGE_REMOVE");

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

TypeData* script_state_load_config(ScriptState* state, const char* config_file)
{
    type_data = type_data_allocate();

    int error = luaL_dofile(state, config_file);
    if (error)
    {
        printf("%s\n", lua_tostring(state, -1));
        return NULL;
    }

    return type_data;
}

Result script_state_run_behavior(ScriptState* state, int function_ref, BehaviorData* data)
{
    lua_rawgeti(state, LUA_REGISTRYINDEX, function_ref);
    int error = lua_pcall(state, 0, 0, 0);
    if (error)
    {
        printf("%s\n", lua_tostring(state, -1));
        return ERROR;
    }

    return INCOMPLETE;
}

