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

You can modify and access nodes using the `NODE` command.

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

