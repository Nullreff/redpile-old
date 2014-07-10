-- redstone.lua - Redstone logic implementation
--
-- Copyright (C) 2014 Ryan Mendivil <ryan@nullreff.net>
--
-- Redistribution and use in source form, with or without modification, are
-- permitted provided that the following conditions are met:
--
-- 1. Redistributions of source code must retain the above copyright notice,
-- this list of conditions and the following disclaimer.
--
-- 2. Neither the name of the Redpile nor the names of its contributors may
-- be used to endorse or promote products derived from this software without
-- specific prior written permission.
--
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
-- AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
-- IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
-- ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
-- LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
-- CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
-- SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
-- INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
-- CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
-- ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
-- POSSIBILITY OF SUCH DAMAGE.
--

print('Loading Redstone types...')
MAX_POWER = 15

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
-- However, be aware of three things:
--   1. Any modifications to the world will not occur until after the current
--   tick has completely finished processing all nodes.  If you need to
--   communicate a change with another node, send it a message.
--   2. All data passed in via `self` and `messages` will change between
--   calls.  Do not store copies as it is liable to change.
--   3. This function may be called multiple times before, during, and after
--   the tick or may be cached and never run again.  Therefore it needs be
--   'pure' and not use any global state in the program.
-- Since multiple behaviors can be added to a single type, the return value
-- determines if the behavior was successful and processing should stop (true)
-- or if it didn't find what it needed and processing should continue (false).
--

define_behavior('push_move', MESSAGE_PUSH + MESSAGE_PULL, function(self, messages)
    if messages.count > 0 then
        message = messages.first()
        self:move(message.value)
        return true
    end
    return false
end)

define_behavior('push_break', MESSAGE_PUSH, function(self, messages)
    if messages.count > 0 then
        self.remove()
    end
    return false
end)

define_behavior('power_wire', MESSAGE_POWER, function(self, messages)
    print('Running power_wire')
    return false
end)

define_behavior('power_conductor', MESSAGE_POWER, function(self, messages)
    print('Running power_conductor')
    return false
end)

define_behavior('power_torch', MESSAGE_POWER, function(self, messages)
    print('Running power_torch')
    return false
end)

define_behavior('power_piston', MESSAGE_POWER, function(self, messages)
    print('Running power_piston')
    return false
end)

define_behavior('power_repeater', MESSAGE_POWER, function(self, messages)
    print('Running power_repeater')
    return false
end)

define_behavior('power_comparator', MESSAGE_POWER, function(self, messages)
    print('Running power_comparator')
    return false
end)

define_behavior('power_switch', MESSAGE_POWER, function(self, messages)
    if self.state == 0 then
        return true
    end

    local behind = self:adjacent(BEHIND)
    self:adjacent(function(node)
        if node.type ~= 'CONDUCTOR' or node.location == behind.location then
            node:send(0, MESSAGE_POWER, MAX_POWER)
        end
    end)

    return true
end)

-- Types are created using the `define_type` function which takes:
--
-- NAME <String>
-- The name used to reference this type.  Should be upper case.
--
-- FIELD_COUNT <Number> (must be positive integer)
-- Number of fields to store on this node.  Current convention is:
--   0: POWER
--   1: DIRECTION
--   2: STATE
-- Other indexes may be used internally but will not be returned when
-- information about a node is queried.
--
-- BEHAVIOR <String> (multiple)
-- Names of behaviors for this type that will be run in the order listed.
--

define_type('AIR', 0)

define_type('INSULATOR', 0,
    'push_move'
)

define_type('WIRE', 1,
    'push_break',
    'power_wire'
)

define_type('CONDUCTOR', 1,
    'push_move',
    'power_conductor'
)

define_type('TORCH', 2,
    'push_break',
    'power_torch'
)

define_type('PISTON', 2,
    'push_break',
    'power_piston'
)

define_type('REPEATER', 3,
    'push_break',
    'power_repeater'
)

define_type('COMPARATOR', 3,
    'push_break',
    'power_comparator'
)

define_type('SWITCH', 3,
    'push_break',
    'power_switch'
)

