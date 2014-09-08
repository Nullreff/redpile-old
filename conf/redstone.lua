-- redstone.lua - Redstone logic implementation
--
-- Copyright (C) 2014 Ryan Mendivil <ryan@nullreff.net>
-- All rights reserved.
--
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions are met:
--
--   * Redistributions of source code must retain the above copyright notice,
--     this list of conditions and the following disclaimer.
--   * Redistributions in binary form must reproduce the above copyright
--     notice, this list of conditions and the following disclaimer in the
--     documentation and/or other materials provided with the distribution.
--   * Neither the name of Redpile nor the names of its contributors may be
--     used to endorse or promote products derived from this software without
--     specific prior written permission.
--
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
-- AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
-- IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
-- ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
-- LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
-- CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
-- SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
-- INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
-- CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
-- ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
-- POSSIBILITY OF SUCH DAMAGE.
--

MAX_POWER = 15
RETRACTED  = 0
RETRACTING = 1
EXTENDED   = 2
EXTENDING  = 3

function has_lower_power(node, messages, power)
    local received_power = messages:source(node.location)
    return received_power == nil or received_power.value < power
end

function msg_power(message)
    return message and message.value or 0
end

-- Behaviors are created using the `define_behavior` function which takes:
--
-- NAME (String)
-- The name is what will be used to reference a behavior later when we attach
-- it to a type.
--
-- MASK <Number> (must be positive integer)
-- The mask determines which message types this behavior listens for.  All
-- possible message types are define as integers which are powers of two.
-- This means they can be combined together with `+` to allow through multiple
-- message types.  Do not list a message type more than once.
--
-- BEHAVIOR (Function)
-- This function takes a reference to the current node and a list of messages
-- received from other nodes.  It can then do whatever calculations it needs
-- to, send messages to other nodes and make modifications to the world.
-- However, be aware of two things:
--   1. Any modifications to the world will not occur until after the current
--   tick has completely finished processing all nodes.  If you need to
--   communicate a change with another node, send it a message.
--   2. This function may be called multiple times before, during, and after
--   the tick or may be cached and never run again.  Therefore it needs be
--   'pure' and not use any global state in the program.
--

define_behavior('push_solid', MESSAGE_PUSH + MESSAGE_PULL, function(node, messages)
    if messages.count > 0 then
        message = messages:first()
        node:move(message.value)
    end
end)

define_behavior('push_breakable', MESSAGE_PUSH, function(node, messages)
    if messages.count > 0 then
        node:remove()
    end
end)

define_behavior('push_piston', MESSAGE_PUSH + MESSAGE_PULL, function(node, messages)
    if messages.count > 0 and node.state == RETRACTED then
        message = messages:first()
        node:move(message.value)
    end
end)

define_behavior('power_wire', MESSAGE_POWER, function(node, messages)
    local covered = node:adjacent(UP).type ~= 'AIR'
    local power_msg = messages:max()
    node.power = power_msg and power_msg.value or 0

    if node.power == 0 then
        return
    end

    node:adjacent_each(DOWN, function(found)
        if found.type == 'CONDUCTOR' and
           has_lower_power(found, messages, node.power)
       then
           found:send(0, MESSAGE_POWER, node.power)
       end
    end)

    if node.power == 1 then
        return
    end

    local wire_power = node.power - 1

    node:adjacent_each(NORTH, SOUTH, EAST, WEST, function(found, dir)
        if found.type == 'AIR' then
            found = found:adjacent(DOWN)
            if found.type ~= 'WIRE' then
                return
            end
        elseif found.type == 'CONDUCTOR' then
            if node:adjacent(direction_left(dir)).type ~= 'WIRE' and
               node:adjacent(direction_right(dir)).type ~= 'WIRE' and
               has_lower_power(found, messages, wire_power)
           then
               found:send(0, MESSAGE_POWER, wire_power)
           end

           if covered then
               return
           end

           found = found:adjacent(UP)
           if found.type ~= 'WIRE' then
               return
           end
        end

        if has_lower_power(found, messages, wire_power) then
            found:send(0, MESSAGE_POWER, wire_power)
        end
    end)
end)

define_behavior('power_conductor', MESSAGE_POWER, function(node, messages)
    node.power = msg_power(messages:max())
    local max_powerd = node.power == MAX_POWER

    node:adjacent_each(function(found)
        if found.type ~= 'CONDUCTOR' and (max_powerd or found.type ~= 'WIRE') then
            if has_lower_power(found, messages, node.power) then
                found:send(0, MESSAGE_POWER, node.power)
            end
        end
    end)
end)

define_behavior('power_torch', MESSAGE_POWER, function(node, messages)
    local new_power = msg_power(messages:source(node:adjacent(BEHIND).location))
    if new_power > 0 then
        node.power = 0
        return
    end
    node.power = MAX_POWER

    local behind = node:adjacent(BEHIND)
    node:adjacent_each(NORTH, SOUTH, EAST, WEST, DOWN, function(found)
        if found.location ~= behind.location then
            found:send(1, MESSAGE_POWER, MAX_POWER)
        end
    end)

    node:adjacent_each(UP, function(found)
        if found.type == 'CONDUCTOR' then
            found:send(1, MESSAGE_POWER, MAX_POWER)
        end
    end)
end)

define_behavior('power_piston', MESSAGE_POWER, function(node, messages)
    local first = node:adjacent(FORWARDS)
    local second = first:adjacent(node.direction)
    local new_power = msg_power(messages:max())

    if new_power == 0 then
        if first.type == 'AIR' and second.type ~= 'AIR' and node.power > 0 then
            node.state = RETRACTING
        else
            node.state = RETRACTED
        end
    else
        if second.type == 'AIR' and first.type ~= 'AIR' and node.power == 0 then
            node.state = EXTENDING
        else
            node.state = EXTENDED
        end
    end

    node.power = new_power

    if node.state == EXTENDING then
        first:send(1, MESSAGE_PUSH, node.direction)
    elseif node.state == RETRACTING then
        second:send(1, MESSAGE_PULL, direction_invert(node.direction))
    end
end)

define_behavior('power_repeater', MESSAGE_POWER, function(node, messages)
    node.power = msg_power(messages:source(node:adjacent(BEHIND).location))
    if node.power ~= 0 and
       messages:source(node:adjacent(RIGHT).location) == nil and
       messages:source(node:adjacent(LEFT).location) == nil
   then
       node:adjacent(FORWARDS):send(node.state + 1, MESSAGE_POWER, MAX_POWER)
   end

end)

define_behavior('power_comparator', MESSAGE_POWER, function(node, messages)
    node.power = msg_power(messages:source(node:adjacent(BEHIND).location))
    if node.power == 0 then
        return
    end

    local side_power = math.max(
        msg_power(messages:source(node:adjacent(LEFT).location)),
        msg_power(messages:source(node:adjacent(RIGHT).location))
    )

    local change = node.power
    if node.state > 0 then
        change = change - side_power
    end

    local new_power = (node.power > side_power) and change or 0
    if new_power ~= 0 then
        node:adjacent(FORWARDS):send(1, MESSAGE_POWER, new_power)
    end
end)

define_behavior('power_switch', 0, function(node, messages)
    if node.state == 0 then
        node.power = 0
        return
    end

    node.power = MAX_POWER
    local behind = node:adjacent(BEHIND)
    node:adjacent_each(function(found)
        if found.type ~= 'CONDUCTOR' or found.location == behind.location then
            found:send(0, MESSAGE_POWER, MAX_POWER)
        end
    end)
end)

-- Types are created using the `define_type` function which takes:
--
-- NAME <String>
-- The name used to reference this type.  Should be upper case.
--
-- FIELDS <Table>
-- Table of fields to store on this node.  Declare in the format:
--   {FIELD_NAME} = {FIELD_TYPE}
-- Available field types are:
--   FIELD_INT
--   FIELD_DIRECTION
--
-- BEHAVIORS <Table>
-- Table containing names of behaviors for this type that will be run in the
-- order listed.
--

define_type(
    'AIR',
    {},
    {}
)

define_type(
    'INSULATOR',
    {},
    {'push_solid'}
)

define_type(
    'WIRE',
    {power = FIELD_INT},
    {'push_breakable', 'power_wire'}
)

define_type(
    'CONDUCTOR',
    {power = FIELD_INT},
    {'push_solid', 'power_conductor'}
)

define_type(
    'TORCH',
    {power = FIELD_INT, direction = FIELD_DIRECTION},
    {'push_breakable', 'power_torch'}
)

define_type(
    'PISTON',
    {power = FIELD_INT, direction = FIELD_DIRECTION, state = FIELD_INT},
    {'push_piston', 'power_piston'}
)

define_type(
    'REPEATER',
    {power = FIELD_INT, direction = FIELD_DIRECTION, state = FIELD_INT},
    {'push_breakable', 'power_repeater'}
)

define_type(
    'COMPARATOR',
    {power = FIELD_INT, direction =  FIELD_DIRECTION, state = FIELD_INT},
    {'push_breakable', 'power_comparator'}
)

define_type(
    'SWITCH',
    {power = FIELD_INT, direction = FIELD_DIRECTION, state = FIELD_INT},
    {'push_breakable', 'power_switch'}
)

