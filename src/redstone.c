/* redstone.c - Redstone logic implementation
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

#include <string.h>
#include <assert.h>
#include "world.h"
#include "redstone.h"

#define MAX_POWER 15

#define LOCATION(NODE)  ((NODE)->location)
#define MATERIAL(NODE)  ((NODE)->type)
#define POWER(NODE)     FIELD_GET(NODE, 0)
#define DIRECTION(NODE) FIELD_GET(NODE, 1)
#define STATE(NODE)     FIELD_GET(NODE, 2)

#define LOWER_POWER(NODE,POWER) messages_power_check(in, LOCATION(NODE), POWER)
#define NODE_ADJACENT(NODE,DIR) world_get_adjacent_node(world, NODE, DIR)
#define MOVE_TO_NODE(NODE,DIR) NODE = NODE_ADJACENT(NODE, DIR)

#define SEND_POWER(NODE,POWER,DELAY) queue_add(messages, MESSAGE_POWER,  world->ticks + (DELAY), node, NODE, POWER)
#define SEND_MOVE(NODE,DIR,DELAY)    queue_add(messages, MESSAGE_MOVE,   world->ticks + (DELAY), node, NODE, DIR  )
#define CMD_POWER(POWER)             queue_add(sets,     MESSAGE_POWER,  world->ticks,           node, node, POWER)
#define CMD_MOVE(DIR)                queue_add(sets,     MESSAGE_MOVE,   world->ticks,           node, node, DIR  )
#define CMD_REMOVE()                 queue_add(sets,     MESSAGE_REMOVE, world->ticks,           node, node, 0    )

TYPE_BEHAVIOR(push_move)
{
    Message* move_inst = messages_find_move(in);
    if (move_inst != NULL)
    {
        CMD_MOVE(move_inst->message);
        return true;
    }
    return false;
}

TYPE_BEHAVIOR(push_break)
{
    Message* move_inst = messages_find_move(in);
    if (move_inst != NULL)
    {
        CMD_REMOVE();
        return true;
    }
    return false;
}

TYPE_BEHAVIOR(EMPTY) { return true; }
TYPE_BEHAVIOR(AIR) { return true; }
TYPE_BEHAVIOR(INSULATOR) { return true; }

TYPE_BEHAVIOR(CONDUCTOR)
{
    if (RUN_BEHAVIOR(push_move))
        return true;

    unsigned int power = messages_max_power(in);
    CMD_POWER(power);
    if (power == 0)
        return true;

    bool max_powered = power == MAX_POWER;

    for (int i = 0; i < DIRECTIONS_COUNT; i++)
    {
        Direction dir = (Direction)i;
        Node* found_node = NODE_ADJACENT(node, dir);
        if (MATERIAL(found_node) == CONDUCTOR)
            continue;

        if (!max_powered && MATERIAL(found_node) == WIRE)
            continue;

        if (LOWER_POWER(found_node, power))
            SEND_POWER(found_node, power, 0);
    }

    return true;
}

TYPE_BEHAVIOR(WIRE)
{
    if (RUN_BEHAVIOR(push_break))
        return true;

    Node* above = NODE_ADJACENT(node, UP);
    bool covered = above != NULL && MATERIAL(above) != AIR && MATERIAL(above) != EMPTY;

    int new_power = messages_max_power(in);
    CMD_POWER(new_power);

    if (new_power == 0)
        return true;

    int wire_power = new_power;
    if (wire_power > 0)
        wire_power--;

    // Adjacent blocks
    Direction directions[4] = {NORTH, SOUTH, EAST, WEST};
    for (int i = 0; i < 4; i++)
    {
        Direction dir = directions[i];
        // Directly adjacent
        Node* found_node = NODE_ADJACENT(node, dir);

        // Down one block
        if (MATERIAL(found_node) == AIR)
        {
            MOVE_TO_NODE(found_node, DOWN);
            if (MATERIAL(found_node) != WIRE)
                continue;
        }
        // Up one block
        else if (MATERIAL(found_node) == CONDUCTOR)
        {
            // Add charge to the block
            Node* right = NODE_ADJACENT(node, direction_left(dir));
            Node* left = NODE_ADJACENT(node, direction_right(dir));

            if (MATERIAL(right) != WIRE && MATERIAL(left) != WIRE &&
                LOWER_POWER(found_node, wire_power))
            {
                SEND_POWER(found_node, wire_power, 0);
            }

            if (covered)
                continue;

            MOVE_TO_NODE(found_node, UP);
            if (MATERIAL(found_node) != WIRE)
                continue;
        }

        if (LOWER_POWER(found_node, wire_power))
            SEND_POWER(found_node, wire_power, 0);
    }

    // Block below
    Node* down_node = NODE_ADJACENT(node, DOWN);
    if (MATERIAL(down_node) == CONDUCTOR && LOWER_POWER(down_node, new_power))
        SEND_POWER(down_node, new_power, 0);

    return true;
}

#define RETRACTED 0
#define RETRACTING 1
#define EXTENDED 2
#define EXTENDING 3
TYPE_BEHAVIOR(PISTON)
{
    Node* first = NODE_ADJACENT(node, DIRECTION(node));
    Node* second = NODE_ADJACENT(first, DIRECTION(node));

    unsigned int new_power = messages_max_power(in);

    unsigned int state;
    if (new_power == 0)
    {
        if (MATERIAL(first) == AIR)
            state = RETRACTING;
        else
            state = RETRACTED;
    }
    else
    {
        if (MATERIAL(second) == AIR)
            state = EXTENDING;
        else
            state = EXTENDED;
    }

    if (state == RETRACTED && RUN_BEHAVIOR(push_move))
        return true;

    CMD_POWER(new_power);

    if (state == EXTENDING)
        SEND_MOVE(first, DIRECTION(node), 1);
    else if (state == RETRACTING)
        SEND_MOVE(second, direction_invert(DIRECTION(node)), 1);

    return true;
}

TYPE_BEHAVIOR(COMPARATOR)
{
    if (RUN_BEHAVIOR(push_break))
        return true;

    unsigned int side_power = 0;
    int new_power = 0;
    Location right  = location_move(LOCATION(node), direction_right(DIRECTION(node)), 1);
    Location left   = location_move(LOCATION(node), direction_left(DIRECTION(node)), 1);
    Location behind = location_move(LOCATION(node), direction_invert(DIRECTION(node)), 1);
    
    for (int i = 0; i < in->size; i++)
    {
        Message* inst = in->data + i;

        // Power coming from the side
        if ((LOCATION_EQUALS(inst->source.location, right) ||
             LOCATION_EQUALS(inst->source.location, left)) &&
            side_power < inst->message)
        {
            side_power = inst->message;
        }

        // Power coming from behind
        if (LOCATION_EQUALS(inst->source.location, behind))
            new_power = inst->message;
    }

    CMD_POWER(new_power);

    if (new_power == 0)
        return true;

    int change = new_power;

    // Subtraction mode
    if (STATE(node) > 0)
        change -= side_power;

    new_power = new_power > side_power ? change : 0;

    if (new_power == 0)
        return true;

    // Pass charge to the wire or conductor in front
    Node* found_node = NODE_ADJACENT(node, DIRECTION(node));
    SEND_POWER(found_node, new_power, 1);

    return true;
}

TYPE_BEHAVIOR(REPEATER)
{
    if (RUN_BEHAVIOR(push_break))
        return true;

    bool side_powered = false;
    int new_power = 0;
    Location right  = location_move(LOCATION(node), direction_right(DIRECTION(node)), 1);
    Location left   = location_move(LOCATION(node), direction_left(DIRECTION(node)), 1);
    Location behind = location_move(LOCATION(node), direction_invert(DIRECTION(node)), 1);
    
    for (int i = 0; i < in->size; i++)
    {
        Message* inst = in->data + i;

        // Power coming from the side
        if (inst->source.type == REPEATER &&
            (LOCATION_EQUALS(inst->source.location, right) ||
            LOCATION_EQUALS(inst->source.location, left)))
        {
            side_powered = (inst->message > 0) || side_powered;
        }

        // Power coming from behind
        if (LOCATION_EQUALS(inst->source.location, behind))
            new_power = inst->message;
    }

    CMD_POWER(new_power);

    if (new_power == 0 || side_powered)
        return true;

    // Pass charge to the wire or conductor in front after a delay
    Node* found_node = NODE_ADJACENT(node, DIRECTION(node));
    SEND_POWER(found_node, MAX_POWER, STATE(node) + 1);

    return true;
}

TYPE_BEHAVIOR(TORCH)
{
    if (RUN_BEHAVIOR(push_break))
        return true;

    unsigned int new_power = 0;
    Location loc_behind = location_move(LOCATION(node), direction_invert(DIRECTION(node)), 1);
    for (int i = 0; i < in->size; i++)
    {
        Message* inst = in->data + i;

        // Power coming from behind
        if (LOCATION_EQUALS(inst->source.location, loc_behind))
            new_power = inst->message;
    }

    CMD_POWER(new_power);

    if (new_power > 0)
        return true;

    // Pass charge to any adjacent wires
    Direction behind = direction_invert(DIRECTION(node));
    Direction directions[5] = {NORTH, SOUTH, EAST, WEST, DOWN};
    for (int i = 0; i < 5; i++)
    {
        Direction dir = directions[i];
        if (dir == behind)
            continue;

        Node* found_node = NODE_ADJACENT(node, dir);
        SEND_POWER(found_node, MAX_POWER, 1);
    }

    // Pass charge up to a conductor
    Node* up_node = NODE_ADJACENT(node, UP);
    if (MATERIAL(up_node) == CONDUCTOR)
        SEND_POWER(up_node, MAX_POWER, 1);

    return true;
}

TYPE_BEHAVIOR(SWITCH)
{
    if (RUN_BEHAVIOR(push_break))
        return true;

    Message* move_inst = messages_find_move(in);
    if (move_inst != NULL)
    {
        CMD_REMOVE();
        return true;
    }

    if (STATE(node) == 0)
        return true;

    Direction behind = direction_invert(DIRECTION(node));
    for (Direction dir = (Direction)0; dir < DIRECTIONS_COUNT; dir++)
    {
        Node* found_node = NODE_ADJACENT(node, dir);
        if (MATERIAL(found_node) != CONDUCTOR || dir == behind)
            SEND_POWER(found_node, MAX_POWER, 0);
    }

    return true;
}

