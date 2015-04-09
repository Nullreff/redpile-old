/* script.c - Scripting engine built on lua
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

#include "script.h"
#include "type.h"
#include "common.h"
#include "repl.h"

#define LUA_ERROR(MESSAGE) lua_pushstring(state, MESSAGE); lua_error(state)
#define LUA_ERROR_IF(CONDITION,MESSAGE) if (CONDITION) { LUA_ERROR(MESSAGE); }
#define IS_UINT(NUM) ((NUM - ((double)(int)NUM) == 0) && (NUM >= 0))

// Used during script_state_load_config and script_state_run_behavior
// during callbacks into C from Lua code
TypeData* type_data = NULL;
ScriptData* script_data = NULL;

// Keeps track of the current node during calls to `adjacent`.
#define NODE_LIST_SIZE 20
NodeList* node_list = NULL;

static void script_create_node(ScriptState* state, Node* node);
static void script_create_message(ScriptState* state, Message* message);

static int script_define_behavior(ScriptState* state)
{
    assert(type_data != NULL);

    LUA_ERROR_IF(!lua_isstring(state, 1), "You must pass a behavior name");
    LUA_ERROR_IF(!lua_istable(state, 2), "You must pass a table of message types");
    LUA_ERROR_IF(!lua_isfunction(state, 3), "You must pass a behavior function");

    char* name = strdup(lua_tostring(state, 1));
    int function_ref = luaL_ref(state, LUA_REGISTRYINDEX);
    unsigned int mask = 0;

    lua_pushnil(state);
    while (lua_next(state, 2) != 0)
    {
        LUA_ERROR_IF(!lua_isstring(state, -1), "Message type must be a string");
        const char* message_name = lua_tostring(state, -1);

        MessageType* message_type = type_data_find_message_type(type_data, message_name);
        if (message_type == NULL)
        {
            char* dup_name = strdup(message_name);
            message_type = type_data_append_message_type(type_data, dup_name);
        }

        mask |= message_type->id;
        lua_pop(state, 1);
    }

    type_data_append_behavior(type_data, name, mask, function_ref);
    return 0;
}

static int script_define_type(ScriptState* state)
{
    assert(type_data != NULL);

    int top = lua_gettop(state);
    LUA_ERROR_IF(top != 3, "define_type requires 3 arguments");
    LUA_ERROR_IF(!lua_isstring(state, 1), "You must pass a type name");
    LUA_ERROR_IF(!lua_istable(state, 2), "You must pass a table of fields");
    LUA_ERROR_IF(!lua_istable(state, 3), "You must pass a list of behaviors");

    char* name = strdup(lua_tostring(state, 1));
    unsigned int field_count;
    unsigned int behavior_count;

    // By default, we allocate room for 100 fields and behaviors
    // You can trim this number down or realloc after filling.
    Type* type = type_data_append_type(type_data, name, MAX_FIELDS, MAX_FIELDS);

    // Behaviors
    lua_pushnil(state);
    for (behavior_count = 0; lua_next(state, 3) != 0; behavior_count++)
    {
        LUA_ERROR_IF(behavior_count >= MAX_FIELDS, "Maximum number of behaviors exceeded");
        const char* name = luaL_checkstring(state, -1);
        Behavior* behavior = type_data_find_behavior(type_data, name);
        ERROR_IF(behavior == NULL, "Could not find behavior");
        type->behaviors->data[behavior_count] = behavior;
        type->behavior_mask |= behavior->mask;
        lua_pop(state, 1);
    }
    type->behaviors->count = behavior_count;

    // Fields
    lua_pushnil(state);
    for (field_count = 0; lua_next(state, 2) != 0; field_count++)
    {
        LUA_ERROR_IF(field_count >= MAX_FIELDS, "Maximum number of fields exceeded");
        char* name = strdup(luaL_checkstring(state, -2));
        double raw_field_type = lua_tonumber(state, -1);
        LUA_ERROR_IF(!IS_UINT(raw_field_type), "Field type");
        type->fields->data[field_count] = field_type_create(name, raw_field_type);
        lua_pop(state, 1);
    }
    type->fields->count = field_count;

    // Set the first passed in as the default type
    if (type_data->type_count == 1)
    {
        LUA_ERROR_IF(field_count > 0, "The default type cannot have any fields");
        LUA_ERROR_IF(behavior_count > 0, "The default type cannot have any behaviors");
        type_data_set_default_type(type_data, type);
    }

    return 0;
}

static int script_direction_right(ScriptState* state)
{
    LUA_ERROR_IF(!lua_isnumber(state, 1), "You must pass a direction to direction_right");
    double raw_direction = lua_tonumber(state, 1);
    LUA_ERROR_IF(!IS_UINT(raw_direction) || raw_direction >= DIRECTIONS_COUNT, "Invalid direction");

    Direction direction = raw_direction;
    Direction inverse = direction_right(direction);
    lua_pushnumber(state, inverse);

    return 1;
}

static int script_direction_left(ScriptState* state)
{
    LUA_ERROR_IF(!lua_isnumber(state, 1), "You must pass a direction to direction_left");
    double raw_direction = lua_tonumber(state, 1);
    LUA_ERROR_IF(!IS_UINT(raw_direction) || raw_direction >= DIRECTIONS_COUNT, "Invalid direction");

    Direction direction = raw_direction;
    Direction inverse = direction_left(direction);
    lua_pushnumber(state, inverse);

    return 1;
}

static int script_direction_invert(ScriptState* state)
{
    LUA_ERROR_IF(!lua_isnumber(state, 1), "You must pass a direction to direction_invert");
    double raw_direction = lua_tonumber(state, 1);
    LUA_ERROR_IF(!IS_UINT(raw_direction) || raw_direction >= DIRECTIONS_COUNT, "Invalid direction");

    Direction direction = raw_direction;
    Direction inverse = direction_invert(direction);
    lua_pushnumber(state, inverse);

    return 1;
}

static Location script_location_from_stack(ScriptState* state, unsigned int stack_index)
{
    luaL_checktype(state, stack_index, LUA_TTABLE);

    lua_getfield(state, stack_index, "x");
    ERROR_IF(lua_isnil(state, -1), "Missing field X");
    ERROR_IF(!lua_isnumber(state, -1), "The field X must be a number");
    int x = lua_tointeger(state, -1);

    lua_getfield(state, stack_index, "y");
    ERROR_IF(lua_isnil(state, -1), "Missing field Y");
    ERROR_IF(!lua_isnumber(state, -1), "The field Y must be a number");
    int y = lua_tointeger(state, -1);

    lua_getfield(state, stack_index, "z");
    ERROR_IF(lua_isnil(state, -1), "Missing field Z");
    ERROR_IF(!lua_isnumber(state, -1), "The field Z must be a number");
    int z = lua_tointeger(state, -1);

    lua_pop(state, 3);
    return location_create(x, y, z);
}

static int script_location_eq(ScriptState* state)
{
    luaL_checktype(state, 1, LUA_TTABLE);
    luaL_checktype(state, 2, LUA_TTABLE);
    lua_getfield(state, 1, "x");
    double x1 = lua_tonumber(state, -1);
    lua_getfield(state, 2, "x");
    double x2 = lua_tonumber(state, -1);
    lua_getfield(state, 1, "y");
    double y1 = lua_tonumber(state, -1);
    lua_getfield(state, 2, "y");
    double y2 = lua_tonumber(state, -1);
    lua_getfield(state, 1, "z");
    double z1 = lua_tonumber(state, -1);
    lua_getfield(state, 2, "z");
    double z2 = lua_tonumber(state, -1);

    lua_pushboolean(state, x1 == x2 && y1 == y2 && z1 == z2);
    return 1;
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

    lua_createtable(state, 0, 1);
    lua_pushcfunction(state, script_location_eq);
    lua_setfield(state, -2, "__eq");
    lua_setmetatable(state, -2);
}

static Node* script_node_from_stack(ScriptState* state, unsigned int stack_index)
{
    luaL_checktype(state, stack_index, LUA_TTABLE);
    lua_getfield(state, stack_index, "stack_index");
    double raw_index = lua_tonumber(state, -1);
    assert(IS_UINT(raw_index));

    Node* current = node_list_index(node_list, (unsigned int)raw_index);

    lua_pop(state, 1);
    return current;
}

static int script_node_adjacent(ScriptState* state)
{
    assert(script_data != NULL);

    Node* current = script_node_from_stack(state, 1);
    int top = lua_gettop(state);

    if (top == 1)
    {
        // Loop through all directions
        for (Direction dir = (Direction)0; dir < DIRECTIONS_COUNT; dir++)
        {
            Node node;
            world_get_adjacent_node(script_data->world, current, dir, &node);
            script_create_node(state, &node);
        }
        return DIRECTIONS_COUNT;
    }
    else
    {
        // Loop through directions passed
        for (int i = 2; i <= top; i++)
        {
            LUA_ERROR_IF(!lua_isnumber(state, i), "You must pass a direction to adjacent");
            double raw_direction = lua_tonumber(state, i);
            LUA_ERROR_IF(!IS_UINT(raw_direction) || raw_direction >= DIRECTIONS_COUNT + MOVEMENTS_COUNT, "Invalid direction");

            Direction direction = raw_direction;
            if (raw_direction >= DIRECTIONS_COUNT)
            {
                unsigned int index;
                Field* field = type_find_field(current->data->type, "direction", &index);
                LUA_ERROR_IF(!field || field->type != FIELD_DIRECTION, "No direction field found on the node passed to adjacent");
                direction = direction_move(FIELD_GET(current, index, direction), (Movement)raw_direction);
            }

            Node node;
            world_get_adjacent_node(script_data->world, current, direction, &node);
            script_create_node(state, &node);
        }

        return top - 1;
    }
}

static int script_node_adjacent_each(ScriptState* state)
{
    assert(script_data != NULL);

    Node* current = script_node_from_stack(state, 1);
    int top = lua_gettop(state);
    int function_ref = luaL_ref(state, LUA_REGISTRYINDEX);

    if (top == 2)
    {
        // Function only, loop through all directions
        for (Direction dir = (Direction)0; dir < DIRECTIONS_COUNT; dir++)
        {
            Node node;
            world_get_adjacent_node(script_data->world, current, dir, &node);
            lua_rawgeti(state, LUA_REGISTRYINDEX, function_ref);
            script_create_node(state, &node);
            lua_pushnumber(state, dir);
            lua_call(state, 2, 0);
        }
    }
    else
    {
        // Loop through all the directions passed
        for (int i = 2; i < top; i++)
        {
            LUA_ERROR_IF(!lua_isnumber(state, i), "You must pass a direction to adjacent");
            double raw_direction = lua_tonumber(state, i);
            LUA_ERROR_IF(!IS_UINT(raw_direction) || raw_direction >= DIRECTIONS_COUNT + MOVEMENTS_COUNT, "Invalid direction");

            Direction direction = raw_direction;
            if (raw_direction >= DIRECTIONS_COUNT)
            {
                unsigned int index;
                Field* field = type_find_field(current->data->type, "direction", &index);
                LUA_ERROR_IF(!field || field->type != FIELD_DIRECTION, "No direction field found on the node passed to adjacent");
                direction = direction_move(FIELD_GET(current, index, direction), (Movement)raw_direction);
            }

            Node node;
            world_get_adjacent_node(script_data->world, current, direction, &node);
            lua_rawgeti(state, LUA_REGISTRYINDEX, function_ref);
            script_create_node(state, &node);
            lua_pushnumber(state, direction);
            lua_call(state, 2, 0);
        }

    }
    return 0;
}

struct script_node_surrounding_each_args {
    ScriptState* state;
    Location center;
    int function_ref;
};

static void script_node_surrounding_each_callback(Location location, Node* node, void* args)
{
    Location center = ((struct script_node_surrounding_each_args*)args)->center;
    if (location_equals(location, center))
        return;

    ScriptState* state = ((struct script_node_surrounding_each_args*)args)->state;
    int function_ref = ((struct script_node_surrounding_each_args*)args)->function_ref;

    lua_rawgeti(state, LUA_REGISTRYINDEX, function_ref);
    script_create_node(state, node);
    lua_call(state, 1, 0);
}

static int script_node_surrounding_each(ScriptState* state)
{
    assert(script_data != NULL);

    Node* current = script_node_from_stack(state, 1);
    int top = lua_gettop(state);
    LUA_ERROR_IF(top != 2, "surrounding_each doesn't currently support the passing of directions");
    int function_ref = luaL_ref(state, LUA_REGISTRYINDEX);

    Region* region = region_allocate(
            (Range){current->location.x - 1, current->location.x + 1, 1},
            (Range){current->location.y - 1, current->location.y + 1, 1},
            (Range){current->location.z - 1, current->location.z + 1, 1});

    struct script_node_surrounding_each_args args = {state, current->location, function_ref};
    world_get_region(script_data->world, region, script_node_surrounding_each_callback, &args);

    return 0;
}

static int script_node_send(ScriptState* state)
{
    assert(script_data != NULL);

    Node* source = node_list_index(node_list, 0);
    Node* target = script_node_from_stack(state, 1);
    assert(source != target);

    LUA_ERROR_IF(!lua_isstring(state, 2), "You must pass a message type to send");
    const char* message_name = lua_tostring(state, 2);
    MessageType* message_type = type_data_find_message_type(
            script_data->world->type_data, message_name);
    LUA_ERROR_IF(!message_type, "Unknown message type");

    LUA_ERROR_IF(!lua_isnumber(state, 3), "You must pass a delay to send");
    double raw_delay = lua_tonumber(state, 3);
    LUA_ERROR_IF(!IS_UINT(raw_delay), "Delay must be greater than or equal to zero");

    LUA_ERROR_IF(!lua_isnumber(state, 4), "You must pass a value to send");
    double raw_value = lua_tonumber(state, 4);
    LUA_ERROR_IF(!IS_UINT(raw_value), "Value must be greater than or equal to zero");

    // Only send messages the target node listens for
    if ((target->data->type->behavior_mask & message_type->id) != 0)
    {
        unsigned int delay = raw_delay;
        FieldValue value = { .integer = raw_value };
        queue_add(
            script_data->messages,
            message_type->id,
            script_data->world->ticks + delay,
            source,
            target,
            0,
            value
        );
    }

    return 0;
}

static int script_node_move(ScriptState* state)
{
    assert(script_data != NULL);

    Node* current = script_node_from_stack(state, 1);

    LUA_ERROR_IF(!lua_isnumber(state, 2), "You must pass a direction to move");
    double raw_direction = lua_tonumber(state, 2);
    LUA_ERROR_IF(!IS_UINT(raw_direction) || raw_direction >= DIRECTIONS_COUNT, "Invalid direction");

    FieldValue value = { .direction = raw_direction };
    queue_add(
        script_data->sets,
        SM_MOVE,
        script_data->world->ticks,
        current,
        current,
        0,
        value
    );

    return 0;
}

static int script_node_remove(ScriptState* state)
{
    assert(script_data != NULL);

    Node* current = script_node_from_stack(state, 1);

    FieldValue value = {};
    queue_add(
        script_data->sets,
        SM_REMOVE,
        script_data->world->ticks,
        current,
        current,
        0,
        value
    );

    return 0;
}

static int script_node_data(ScriptState* state)
{
    assert(script_data != NULL);

    Node* current = script_node_from_stack(state, 1);

    LUA_ERROR_IF(!lua_isstring(state, 2), "You must pass a message to data");
    char* message = strdup(lua_tostring(state, 2));

    FieldValue value = { .string = message };
    queue_add(
        script_data->sets,
        SM_DATA,
        script_data->world->ticks,
        current,
        current,
        0,
        value
    );

    return 0;
}

static int script_node_index_get(ScriptState* state)
{
    assert(script_data != NULL);

    const char* field = lua_tostring(state, 2);
    lua_getmetatable(state, 1);
    lua_pushstring(state, field);
    lua_gettable(state, -2);
    return !lua_isnil(state, -1) ? 1 : 0;
}

static int script_node_index_set(ScriptState* state)
{
    assert(script_data != NULL);

    Node* node = script_node_from_stack(state, 1);
    const char* name = lua_tostring(state, 2);

    unsigned int index;
    Field* field = type_find_field(node->data->type, name, &index);
    LUA_ERROR_IF(!field, "Could not find field");

    FieldValue value;
    switch (field->type)
    {
        case FIELD_INTEGER: {
            LUA_ERROR_IF(!lua_isnumber(state, 3),
                         "Attempting to assign a non number to a numeric field");
            value.integer = lua_tonumber(state, 3);
        } break;

        case FIELD_DIRECTION: {
            LUA_ERROR_IF(!lua_isnumber(state, 3),
                         "Attempting to assign a non number to a direction field");
            value.direction = lua_tonumber(state, 3);
            LUA_ERROR_IF(value.direction >= DIRECTIONS_COUNT,
                         "Attempting to assign a non direction number to a direction field");
        } break;

        case FIELD_STRING: {
            LUA_ERROR_IF(!lua_isstring(state, 3),
                         "Attempting to assign a non string to a string field");
            value.string = strdup(lua_tostring(state, 3));
        } break;

        default:
            ERROR("Unknown field type");
    }

    queue_add(
        script_data->sets,
        SM_FIELD,
        script_data->world->ticks,
        node,
        node,
        index,
        value
    );

    // Save the new value in the metatable
    lua_getmetatable(state, 1);
    lua_pushstring(state, name);
    switch (field->type)
    {
        case FIELD_INTEGER:
            lua_pushnumber(state, value.integer);
            break;

        case FIELD_DIRECTION:
            lua_pushnumber(state, value.direction);
            break;

        case FIELD_STRING:
            lua_pushstring(state, value.string);
            break;
    }
    lua_settable(state, -3);

    return 0;
}

static void script_create_node(ScriptState* state, Node* node)
{
    assert(node != NULL);

    // Node table
    lua_createtable(state, 0, 6);

    static const luaL_Reg node_funcs[] = {
        {"adjacent", script_node_adjacent},
        {"adjacent_each", script_node_adjacent_each},
        {"surrounding_each", script_node_surrounding_each},
        {"send", script_node_send},
        {"move", script_node_move},
        {"remove", script_node_remove},
        {"data", script_node_data},
        {NULL, NULL}
    };
    luaL_setfuncs(state, node_funcs, 0);

    // Node metatable
    unsigned int field_count = node->data->type->fields->count;
    lua_createtable(state, 0, 5 + field_count);

    unsigned int index = node_list_add(node_list, node);

    lua_pushstring(state, "stack_index");
    lua_pushnumber(state, index);
    lua_settable(state, -3);

    lua_pushstring(state, "location");
    script_create_location(state, node->location);
    lua_settable(state, -3);

    lua_pushstring(state, "type");
    lua_pushstring(state, node->data->type->name);
    lua_settable(state, -3);

    lua_pushcfunction(state, script_node_index_get);
    lua_setfield(state, -2, "__index");

    lua_pushcfunction(state, script_node_index_set);
    lua_setfield(state, -2, "__newindex");

    for (unsigned int i = 0; i < field_count; i++)
    {
        Field* field = node->data->type->fields->data + i;
        lua_pushstring(state, field->name);
        switch (field->type)
        {
            case FIELD_INTEGER:
                lua_pushnumber(state, FIELD_GET(node, i, integer));
                break;

            case FIELD_DIRECTION:
                lua_pushnumber(state, FIELD_GET(node, i, direction));
                break;

            case FIELD_STRING:
                lua_pushstring(state, FIELD_GET(node, i, string));
                break;
        }
        lua_settable(state, -3);
    }

    lua_setmetatable(state, -2);
}

static int script_messages_first(ScriptState* state)
{
    assert(script_data != NULL);

    Message* message = messages_find_first(script_data->input);
    if (message != NULL)
        script_create_message(state, message);
    else
        lua_pushnil(state);

    return 1;
}

static int script_messages_max(ScriptState* state)
{
    assert(script_data != NULL);

    Message* message = messages_find_max(script_data->input);
    if (message != NULL)
        script_create_message(state, message);
    else
        lua_pushnil(state);

    return 1;
}

static int script_messages_source(ScriptState* state)
{
    assert(script_data != NULL);

    Location location;
    if (lua_isnumber(state, 2))
    {
        double raw_direction = lua_tonumber(state, 2);
        LUA_ERROR_IF(!IS_UINT(raw_direction) ||
                     raw_direction >= DIRECTIONS_COUNT + MOVEMENTS_COUNT ||
                     raw_direction < 0,
                     "Invalid direction");
        if (raw_direction < DIRECTIONS_COUNT)
        {
            location = location_move(script_data->node->location,
                                     (Direction)(int)raw_direction, 1);
        }
        else
        {
            unsigned int index;
            Field* field = type_find_field(script_data->node->data->type, "direction", &index);
            LUA_ERROR_IF(!field || field->type != FIELD_DIRECTION, "No direction field found on the node passed to source");
            Direction dir = direction_move(FIELD_GET(script_data->node, index, direction),
                                           (Movement)(int)raw_direction);
            location = location_move(script_data->node->location, dir, 1);
        }
    }
    else
    {
        location = script_location_from_stack(state, 2);
    }

    Message* message = messages_find_source(script_data->input, location);
    if (message != NULL)
        script_create_message(state, message);
    else
        lua_pushnil(state);

    return 1;
}

static int script_messages_each(ScriptState* state)
{
    assert(script_data != NULL);

    int function_ref = luaL_ref(state, LUA_REGISTRYINDEX);

    for (unsigned int i = 0; i < script_data->input->size; i++)
    {
        Message* message = script_data->input->data + i;
        lua_rawgeti(state, LUA_REGISTRYINDEX, function_ref);
        script_create_message(state, message);
        lua_call(state, 1, 0);
    }

    return 0;
}

static void script_create_message(ScriptState* state, Message* message)
{
    lua_createtable(state, 0, 2);

    lua_pushstring(state, "value");
    lua_pushnumber(state, message->value);
    lua_settable(state, -3);

    lua_pushstring(state, "source");
    script_create_location(state, message->source.location);
    lua_settable(state, -3);
}

static void script_setup_data(ScriptState* state, ScriptData* data)
{
    //self
    script_create_node(state, data->node);

    // messages
    lua_createtable(state, 0, 3);

    lua_pushstring(state, "count");
    lua_pushnumber(state, data->input->size);
    lua_settable(state, -3);

    static const luaL_Reg message_funcs[] = {
        {"first", script_messages_first},
        {"max", script_messages_max},
        {"source", script_messages_source},
        {"each", script_messages_each},
        {NULL, NULL}
    };
    luaL_setfuncs(state, message_funcs, 0);
}

ScriptState* script_state_allocate(void)
{
    lua_State* state = luaL_newstate();

    luaL_openlibs(state);

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

    lua_pushnumber(state, FORWARDS);
    lua_setglobal(state, "FORWARDS");
    lua_pushnumber(state, BEHIND);
    lua_setglobal(state, "BEHIND");
    lua_pushnumber(state, LEFT);
    lua_setglobal(state, "LEFT");
    lua_pushnumber(state, RIGHT);
    lua_setglobal(state, "RIGHT");

    lua_pushnumber(state, FIELD_INTEGER);
    lua_setglobal(state, "FIELD_INTEGER");
    lua_pushnumber(state, FIELD_DIRECTION);
    lua_setglobal(state, "FIELD_DIRECTION");
    lua_pushnumber(state, FIELD_STRING);
    lua_setglobal(state, "FIELD_STRING");

    lua_createtable(state, 0, 5);
    static const luaL_Reg redpile_funcs[] = {
        {"behavior", script_define_behavior},
        {"type", script_define_type},
        {"direction_left", script_direction_left},
        {"direction_right", script_direction_right},
        {"direction_invert", script_direction_invert},
        {NULL, NULL}
    };
    luaL_setfuncs(state, redpile_funcs, 0);
    lua_setglobal(state, "redpile");

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
        repl_print_error("%s\n", lua_tostring(state, -1));
        return NULL;
    }

    if (data->type_count == 0)
    {
        repl_print_error("No types defined in configuration file %s\n", config_file);
        type_data_free(data);
        return NULL;
    }

    return data;
}

bool script_state_run_behavior(ScriptState* state, Behavior* behavior, ScriptData* data)
{
    lua_settop(state, 0);

    lua_rawgeti(state, LUA_REGISTRYINDEX, behavior->function_ref);

    node_list = node_list_allocate(NODE_LIST_SIZE);
    script_setup_data(state, data);

    script_data = data;
    int error = lua_pcall(state, 2, 1, 0);
    script_data = NULL;
    node_list_free(node_list);

    if (error)
    {
        repl_print_error("%s\n", lua_tostring(state, -1));
        return false;
    }

    return true;
}

