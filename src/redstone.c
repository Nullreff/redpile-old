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

#include "redstone.h"
#include "redpile.h"

#define MAX_POWER 15

#define LOCATION(NODE)  ((NODE)->location)
#define MATERIAL(NODE)  ((NODE)->type)
#define POWER(NODE)     FIELD_GET(NODE, 0)
#define DIRECTION(NODE) FIELD_GET(NODE, 1)
#define STATE(NODE)     FIELD_GET(NODE, 2)

#define LOWER_POWER(NODE,POWER) messages_power_check(data->in, LOCATION(NODE), POWER)
#define NODE_ADJACENT(NODE,DIR) world_get_adjacent_node(data->world, NODE, DIR)
#define MOVE_TO_NODE(NODE,DIR) NODE = NODE_ADJACENT(NODE, DIR)

#define SEND(NODE,CMD,VALUE,DELAY) queue_add(data->messages, CMD,            data->world->ticks + (DELAY), data->node, NODE, VALUE)
#define CMD_POWER(POWER)           queue_add(data->sets,     MESSAGE_POWER,  data->world->ticks,           data->node, data->node, POWER)
#define CMD_MOVE(DIR)              queue_add(data->sets,     MESSAGE_PUSH,   data->world->ticks,           data->node, data->node, DIR  )
#define CMD_REMOVE()               queue_add(data->sets,     MESSAGE_REMOVE, data->world->ticks,           data->node, data->node, 0    )

#define AIR        (data->world->types->data + 0)
#define INSULATOR  (data->world->types->data + 1)
#define WIRE       (data->world->types->data + 2)
#define CONDUCTOR  (data->world->types->data + 3)
#define TORCH      (data->world->types->data + 4)
#define PISTON     (data->world->types->data + 5)
#define REPEATER   (data->world->types->data + 6)
#define COMPARATOR (data->world->types->data + 7)
#define SWITCH     (data->world->types->data + 8)

TYPE_BEHAVIOR(PUSH_MOVE)
{
    Message* move_inst = messages_find_first(data->in, MESSAGE_PUSH | MESSAGE_PULL);
    if (move_inst != NULL)
    {
        CMD_MOVE(move_inst->message);
        return true;
    }
    return false;
}

TYPE_BEHAVIOR(PUSH_BREAK)
{
    Message* move_inst = messages_find_first(data->in, MESSAGE_PUSH);
    if (move_inst != NULL)
    {
        CMD_REMOVE();
        return true;
    }
    return false;
}

TYPE_BEHAVIOR(WIRE)
{
    Node* above = NODE_ADJACENT(data->node, UP);
    bool covered = above != NULL && MATERIAL(above) != AIR && MATERIAL(above) != EMPTY;

    int new_power = messages_max_power(data->in);
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
        Node* found_node = NODE_ADJACENT(data->node, dir);

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
            Node* right = NODE_ADJACENT(data->node, direction_left(dir));
            Node* left = NODE_ADJACENT(data->node, direction_right(dir));

            if (MATERIAL(right) != WIRE && MATERIAL(left) != WIRE &&
                LOWER_POWER(found_node, wire_power))
            {
                SEND(found_node, MESSAGE_POWER, wire_power, 0);
            }

            if (covered)
                continue;

            MOVE_TO_NODE(found_node, UP);
            if (MATERIAL(found_node) != WIRE)
                continue;
        }

        if (LOWER_POWER(found_node, wire_power))
            SEND(found_node, MESSAGE_POWER, wire_power, 0);
    }

    // Block below
    Node* down_node = NODE_ADJACENT(data->node, DOWN);
    if (MATERIAL(down_node) == CONDUCTOR && LOWER_POWER(down_node, new_power))
        SEND(down_node, MESSAGE_POWER, new_power, 0);

    return true;
}

TYPE_BEHAVIOR(CONDUCTOR)
{
    unsigned int power = messages_max_power(data->in);
    CMD_POWER(power);
    if (power == 0)
        return true;

    bool max_powered = power == MAX_POWER;

    for (int i = 0; i < DIRECTIONS_COUNT; i++)
    {
        Direction dir = (Direction)i;
        Node* found_node = NODE_ADJACENT(data->node, dir);
        if (MATERIAL(found_node) == CONDUCTOR)
            continue;

        if (!max_powered && MATERIAL(found_node) == WIRE)
            continue;

        if (LOWER_POWER(found_node, power))
            SEND(found_node, MESSAGE_POWER, power, 0);
    }

    return true;
}

TYPE_BEHAVIOR(TORCH)
{
    unsigned int new_power = 0;
    Location loc_behind = location_move(LOCATION(data->node), direction_invert(DIRECTION(data->node)), 1);
    for (int i = 0; i < data->in->size; i++)
    {
        Message* inst = data->in->data + i;

        // Power coming from behind
        if (LOCATION_EQUALS(inst->source.location, loc_behind))
            new_power = inst->message;
    }

    CMD_POWER(new_power);

    if (new_power > 0)
        return true;

    // Pass charge to any adjacent wires
    Direction behind = direction_invert(DIRECTION(data->node));
    Direction directions[5] = {NORTH, SOUTH, EAST, WEST, DOWN};
    for (int i = 0; i < 5; i++)
    {
        Direction dir = directions[i];
        if (dir == behind)
            continue;

        Node* found_node = NODE_ADJACENT(data->node, dir);
        SEND(found_node, MESSAGE_POWER, MAX_POWER, 1);
    }

    // Pass charge up to a conductor
    Node* up_node = NODE_ADJACENT(data->node, UP);
    if (MATERIAL(up_node) == CONDUCTOR)
        SEND(up_node, MESSAGE_POWER, MAX_POWER, 1);

    return true;
}

#define RETRACTED 0
#define RETRACTING 1
#define EXTENDED 2
#define EXTENDING 3
TYPE_BEHAVIOR(PISTON)
{
    Node* first = NODE_ADJACENT(data->node, DIRECTION(data->node));
    Node* second = NODE_ADJACENT(first, DIRECTION(data->node));

    unsigned int new_power = messages_max_power(data->in);

    unsigned int state;
    if (new_power == 0)
    {
        if (MATERIAL(first) == AIR && MATERIAL(second) != AIR && POWER(data->node) != 0)
            state = RETRACTING;
        else
            state = RETRACTED;
    }
    else
    {
        if (MATERIAL(second) == AIR && MATERIAL(first) != AIR && POWER(data->node) == 0)
            state = EXTENDING;
        else
            state = EXTENDED;
    }

    if (state == RETRACTED)
        return false;

    CMD_POWER(new_power);

    if (state == EXTENDING)
        SEND(first, MESSAGE_PUSH, DIRECTION(data->node), 1);
    else if (state == RETRACTING)
        SEND(second, MESSAGE_PULL, direction_invert(DIRECTION(data->node)), 1);

    return true;
}

TYPE_BEHAVIOR(REPEATER)
{
    bool side_powered = false;
    int new_power = 0;
    Location right  = location_move(LOCATION(data->node), direction_right(DIRECTION(data->node)), 1);
    Location left   = location_move(LOCATION(data->node), direction_left(DIRECTION(data->node)), 1);
    Location behind = location_move(LOCATION(data->node), direction_invert(DIRECTION(data->node)), 1);
    
    for (int i = 0; i < data->in->size; i++)
    {
        Message* inst = data->in->data + i;

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
    Node* found_node = NODE_ADJACENT(data->node, DIRECTION(data->node));
    SEND(found_node, MESSAGE_POWER, MAX_POWER, STATE(data->node) + 1);

    return true;
}

TYPE_BEHAVIOR(COMPARATOR)
{
    unsigned int side_power = 0;
    int new_power = 0;
    Location right  = location_move(LOCATION(data->node), direction_right(DIRECTION(data->node)), 1);
    Location left   = location_move(LOCATION(data->node), direction_left(DIRECTION(data->node)), 1);
    Location behind = location_move(LOCATION(data->node), direction_invert(DIRECTION(data->node)), 1);
    
    for (int i = 0; i < data->in->size; i++)
    {
        Message* inst = data->in->data + i;

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
    if (STATE(data->node) > 0)
        change -= side_power;

    new_power = new_power > side_power ? change : 0;

    if (new_power == 0)
        return true;

    // Pass charge to the wire or conductor in front
    Node* found_node = NODE_ADJACENT(data->node, DIRECTION(data->node));
    SEND(found_node, MESSAGE_POWER, new_power, 1);

    return true;
}

TYPE_BEHAVIOR(SWITCH)
{
    if (STATE(data->node) == 0)
        return true;

    Direction behind = direction_invert(DIRECTION(data->node));
    for (Direction dir = (Direction)0; dir < DIRECTIONS_COUNT; dir++)
    {
        Node* found_node = NODE_ADJACENT(data->node, dir);
        if (MATERIAL(found_node) != CONDUCTOR || dir == behind)
            SEND(found_node, MESSAGE_POWER, MAX_POWER, 0);
    }

    return true;
}

