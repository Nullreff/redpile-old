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

// Used during script_state_load_config and script_state_run_behavior
// during callbacks into C from Lua code
TypeData* type_data = NULL;
ScriptData* script_data = NULL;

// Keeps track of the current node during calls to `adjacent`.
#define NODE_STACK_SIZE 20
NodeStack* node_stack = NULL;

static void script_create_node(ScriptState* state, Node* node);

static int script_define_behavior(ScriptState* state)
{
    assert(type_data != NULL);

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

static int script_define_type(ScriptState* state)
{
    assert(type_data != NULL);

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

static int script_direction_invert(ScriptState* state)
{
    LUA_ERROR_IF(!lua_isnumber(state, 1), "You must pass a direction to invert");
    double raw_direction = lua_tonumber(state, 1);
    LUA_ERROR_IF(!IS_UINT(raw_direction) || raw_direction >= DIRECTIONS_COUNT, "Invalid direction");

    Direction direction = raw_direction;
    Direction inverse = direction_invert(direction);
    lua_pushnumber(state, inverse);

    return 1;
}

static void script_create_message(ScriptState* state, Message* message)
{
    lua_createtable(state, 0, 1);

    lua_pushstring(state, "value");
    lua_pushnumber(state, message->value);
    lua_settable(state, -3);
}

static void script_create_location(ScriptState* state, Location location)
{
    lua_createtable(state, 0, 3);

    lua_pushstring(state, "x");
    lua_pushnumber(state, location.x);
    lua_settable(state, -3);

    lua_pushstring(state, "y");
    lua_pushnumber(state, location.y);
    lua_settable(state, -3);

    lua_pushstring(state, "z");
    lua_pushnumber(state, location.z);
    lua_settable(state, -3);
}

static Node* script_node_pop_current(ScriptState* state)
{
    luaL_checktype(state, 1, LUA_TTABLE);
    lua_getfield(state, 1, "index");
    double raw_index = lua_tonumber(state, -1);
    assert(IS_UINT(raw_index));

    Node* current = node_stack_index(node_stack, (unsigned int)raw_index);
    assert(current != NULL);

    lua_pop(state, 1);
    return current;
}

static int script_node_adjacent(ScriptState* state)
{
    assert(script_data != NULL);

    Node* current = script_node_pop_current(state);

    int top = lua_gettop(state);
    LUA_ERROR_IF(top == 1 || !lua_isfunction(state, top), "You must pass a function to adjacent")

    int function_ref = luaL_ref(state, LUA_REGISTRYINDEX);

    if (top == 2)
    {
        // Function only, loop through all directions
        for (Direction dir = (Direction)0; dir < DIRECTIONS_COUNT; dir++)
        {
            Node* node = world_get_adjacent_node(script_data->world, current, dir);
            lua_rawgeti(state, LUA_REGISTRYINDEX, function_ref);
            script_create_node(state, node);
            lua_call(state, 1, 0);
            node_stack_pop(node_stack);
        }
    }
    else
    {
        // Loop through all the directions passed
        for (int i = 2; i < top; i++)
        {
            LUA_ERROR_IF(!lua_isnumber(state, i), "You must pass a direction to adjacent");
            double raw_direction = lua_tonumber(state, i);
            LUA_ERROR_IF(!IS_UINT(raw_direction) || raw_direction >= DIRECTIONS_COUNT, "Invalid direction");
            Direction dir = raw_direction;

            Node* node = world_get_adjacent_node(script_data->world, current, dir);
            lua_rawgeti(state, LUA_REGISTRYINDEX, function_ref);
            script_create_node(state, node);
            lua_call(state, 1, 0);
            node_stack_pop(node_stack);
        }

    }
    return 0;
}

static int script_node_send(ScriptState* state)
{
    assert(script_data != NULL);

    Node* source = node_stack_first(node_stack);
    Node* target = script_node_pop_current(state);
    assert(source != target);

    LUA_ERROR_IF(!lua_isnumber(state, 2), "You must pass a delay to send");
    double raw_delay = lua_tonumber(state, 2);
    LUA_ERROR_IF(!IS_UINT(raw_delay), "Delay must be greater than or equal to zero");

    LUA_ERROR_IF(!lua_isnumber(state, 3), "You must pass a message type to send");
    double raw_message_type = lua_tonumber(state, 3);
    LUA_ERROR_IF(!IS_UINT(raw_message_type), "Message type must be greater than or equal to zero");

    LUA_ERROR_IF(!lua_isnumber(state, 4), "You must pass a value to send");
    double raw_value = lua_tonumber(state, 4);
    LUA_ERROR_IF(!IS_UINT(raw_value), "Value must be greater than or equal to zero");

    unsigned int delay = raw_delay;
    MessageType message_type = raw_message_type;
    unsigned int value = raw_value;

    queue_add(
        script_data->messages,
        message_type,
        script_data->world->ticks + delay,
        source,
        target,
        value
    );

    return 0;
}

static int script_node_move(ScriptState* state)
{
    assert(script_data != NULL);

    Node* current = script_node_pop_current(state);

    LUA_ERROR_IF(!lua_isnumber(state, 2), "You must pass a direction to move");
    double raw_direction = lua_tonumber(state, 2);
    LUA_ERROR_IF(!IS_UINT(raw_direction) || raw_direction >= DIRECTIONS_COUNT, "Invalid direction");

    Direction direction = raw_direction;
    queue_add(
        script_data->sets,
        MESSAGE_PUSH,
        script_data->world->ticks,
        current,
        current,
        direction
    );

    return 0;
}

static int script_node_remove(ScriptState* state)
{
    assert(script_data != NULL);

    Node* current = script_node_pop_current(state);

    queue_add(
        script_data->sets,
        MESSAGE_REMOVE,
        script_data->world->ticks,
        current,
        current,
        0
    );

    return 0;
}

static void script_create_node(ScriptState* state, Node* node)
{
    lua_createtable(state, 0, 10);

    lua_pushstring(state, "location");
    script_create_location(state, node->location);
    lua_settable(state, -3);

    lua_pushstring(state, "type");
    lua_pushstring(state, node->type != EMPTY ? node->type->name : "EMPTY");
    lua_settable(state, -3);

    lua_pushstring(state, "power");
    lua_pushnumber(state, FIELD_GET(node, 0));
    lua_settable(state, -3);

    lua_pushstring(state, "direction");
    lua_pushnumber(state, FIELD_GET(node, 1));
    lua_settable(state, -3);

    lua_pushstring(state, "state");
    lua_pushnumber(state, FIELD_GET(node, 2));
    lua_settable(state, -3);

    int index = node_stack_push(node_stack, node);
    LUA_ERROR_IF(index == -1, "Node stack overflow!");

    lua_pushstring(state, "index");
    lua_pushnumber(state, index);
    lua_settable(state, -3);

    static const luaL_Reg node_funcs[] = {
        {"adjacent", script_node_adjacent},
        {"send", script_node_send},
        {"move", script_node_move},
        {"remove", script_node_remove},
        {NULL, NULL}
    };
    luaL_setfuncs(state, node_funcs, 0);
}

static int script_messages_first(ScriptState* state)
{
    assert(script_data != NULL);

    Message* message = messages_find_first(script_data->input);
    script_create_message(state, message);
    return 1;
}

static void script_setup_data(ScriptState* state, ScriptData* data)
{
    //self
    script_create_node(state, data->node);

    // messages
    lua_createtable(state, 0, 2);

    lua_pushstring(state, "count");
    lua_pushnumber(state, data->input->size);
    lua_settable(state, -3);

    static const luaL_Reg message_funcs[] = {
        {"first", script_messages_first},
        {NULL, NULL}
    };
    luaL_setfuncs(state, message_funcs, 0);
}

ScriptState* script_state_allocate(void)
{
    lua_State* state = luaL_newstate();

    luaopen_base(state);
    luaopen_bit32(state);

    lua_pushnumber(state, NORTH);
    lua_setglobal(state, "NORTH");
    lua_pushnumber(state, SOUTH);
    lua_setglobal(state, "SOUTH");
    lua_pushnumber(state, EAST);
    lua_setglobal(state, "EAST");
    lua_pushnumber(state, WEST);
    lua_setglobal(state, "WEST");
    lua_pushnumber(state, UP);
    lua_setglobal(state, "UP");
    lua_pushnumber(state, DOWN);
    lua_setglobal(state, "DOWN");

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
    lua_pushcfunction(state, script_direction_invert);
    lua_setglobal(state, "direction_invert");

    lua_settop(state, 0);

    return state;
}

void script_state_free(ScriptState* state)
{
    lua_close(state);
}

TypeData* script_state_load_config(ScriptState* state, const char* config_file)
{
    TypeData* data = type_data_allocate();

    type_data = data;
    int error = luaL_dofile(state, config_file);
    type_data = NULL;

    if (error)
    {
        printf("%s\n", lua_tostring(state, -1));
        return NULL;
    }

    return data;
}

Result script_state_run_behavior(ScriptState* state, Behavior* behavior, ScriptData* data)
{
    lua_rawgeti(state, LUA_REGISTRYINDEX, behavior->function_ref);

    node_stack = node_stack_allocate(NODE_STACK_SIZE);
    script_setup_data(state, data);

    script_data = data;
    int error = lua_pcall(state, 2, 1, 0);
    script_data = NULL;
    node_stack_free(node_stack);

    if (error)
    {
        printf("%s\n", lua_tostring(state, -1));
        return ERROR;
    }

    if (!lua_isboolean(state, -1))
    {
        printf("Call to behavior '%s' did not return a boolan\n", behavior->name);
        return ERROR;
    }

    return lua_toboolean(state, -1) ? COMPLETE : INCOMPLETE;
}

