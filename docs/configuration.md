Configuration
=============

Redpile is configured using the [Lua](http://www.lua.org/start.html) programming language.
An example configuration file for Redstone is available in `conf/redstone.lua`.
Before you can start programming, you need to understand how Redpile works internally.

Nodes
-----

Redpile sees the world as a 3D grid of 'nodes' starting at `0,0,0`.
These nodes continue outwards on all three axes both positively and negatively.
Each node has a 'type' assigned to it that determines what data it stores and
how it interacts with other nodes.
Depending on it's type, nodes will have fields that can store data.
For instance, `TORCH` has the fields `power` which stores an integer and
`direction` which stores a direction.

Types
-----

Types are made up of three parts:

1. A distinct name used for identification
2. A list of fields that it should store
3. A list of behaviors that it exhibits

Fields can be one of three different types:

* INTEGER
* DIRECTION
* STRING

Types are declared using the `redpile.type` function:

~~~lua
redpile.type(
    'REPEATER',
    {power = FIELD_INTEGER, direction = FIELD_DIRECTION, state = FIELD_INTEGER},
    {'push_breakable', 'power_repeater'}
)
~~~

Behaviors
---------

In the world of Redpile, you can dictate how nodes interact with one another using 'behaviors'.
A behavior consists of:

1. A distinct name used for identification
2. A list of message types it listens for
3. A Lua function containing the logic to use

Use the `redpile.behavior` function which takes the current node and any messages it has received:

~~~lua
redpile.behavior('power_repeater', {'POWER'}, function(node, messages)
    node.power = value_or_zero(messages:source(BEHIND))
    if node.power ~= 0 and
       messages:source(RIGHT) == nil and
       messages:source(LEFT) == nil
   then
       node:adjacent(FORWARDS):send('POWER', node.state + 1, MAX_POWER)
   end
end)
~~~

When writing a behavior, you have access to the following functions:

### node:adjacent

Syntax: `node:adjacent()`

Returns all nodes adjacent to the current one.

Syntax: `node:adjacent(direction1, direction2, ...)`

Returns all nodes adjacent to the current one in the directions listed.

### node:adjacent_each

Syntax: `node:adjacent_each(callback)`

Calls `callback` for each node adjacent to the current one.

Syntax: `node:adjacent_each(direction1, direction2, ..., function)`

Calls `function` for each node adjacent to the current one in the directions listed.

### node:send

Syntax: `node:send(message, delay, value)`

Sends the message `message` with value `value` to the specified node after `delay` ticks.
`message` is a string and `value` is an integer.

### node:move

Syntax: `node:move(direction)`

Moves the node one node over in `direction`.
If there is a node in it's new location, it will be overwritten.
The nodes existing location will be replaced with a node of the default type.

### node:remove

Syntax: `node:remove()`

Removes any data associated with the current node and sets it to the default type.

### node:data

Syntax: `node:data(message)`

Prints `message` in the format `x,y,z DATA "message"`

### node.location

Syntax `local loc = node.location`

Return a lua table with the `x`, `y` and `z` coordinates of this node.
Assigning a value back does not move this node, use `node:move` instead.

### node.type

Syntax `local type = node.type`

Return the type of this node as a string.
Assigning a value back does not change the type of this node.

### node.[field]

Syntax: `local power = node.power`

Returns the value of `[field]` on this node.

Syntax: `node.power = 15`

Assignes the value provided to the field on this node.
This assignment will be valid during the current behavior and after the entire tick has finished processing.
Do not depend on accessing updated fields from other nodes, please use message passing instead.

### messages.count

Syntax: `local count = messages.count`

Returns the number of messages passed to this behavior.
Assigning a value back to count does nothing.

### messages:first

Syntax: `messages:first()`

Returns the first message passed to this node.
The ordering of messages is indeterminate, please don't rely on it.

### messages:max

Syntax: `messages:max()`

Returns the message with the largest `value` attached to it.
If there are two messages with the same large `value`, the first will be selected.
As with calls to `first`, the ordering of messages is indeterminate.

### messages:source

Syntax: `messages:source(location)`

Returns the message passed from `location`.
If there are two messages from the same location, the first will be selected.
As with calls to `first`, the ordering of messages is indeterminate.

### messages:each

Syntax: `message:each(callback)`

Calls `callback` for every message passed to this behavior.

### redpile.direction_left

Syntax: `redpile.direction_left(direction)`

Returns the direction to the left of `direction`.

* `NORTH` => `WEST`
* `SOUTH` => `EAST`
* `EAST` => `NORTH`
* `WEST` => `SOUTH`
* `UP` => Error
* `DOWN` => Error

### redpile.direction_right

Syntax: `redpile.direction_right(direction)`

Returns the direction to the right of `direction`.

* `NORTH` => `EAST`
* `SOUTH` => `WEST`
* `EAST` => `SOUTH`
* `WEST` => `NORTH`
* `UP` => Error
* `DOWN` => Error

### redpile.direction_invert

Syntax: `redpile.direction_invert(direction)`

Returns the direction inverse to `direction`.

* `NORTH` => `SOUTH`
* `SOUTH` => `NORTH`
* `EAST` => `WEST`
* `WEST` => `EAST`
* `UP` => `DOWN`
* `DOWN` => `UP`

Messages
--------

As mentioned in the Behaviors section, you can communicate between nodes by passing 'messages'.
On the senders side, this is done using the method `node:send`.
On the receiving side, messages are retreived from the `messages` variable passed to a behavior.
When received, messages will contain the following fields:

* `value` (number) - The value sent from `node:send`

