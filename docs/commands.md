Commands
========

Most commands in redpile use a 'region' syntax to designate which nodes should be affected by the command.
Regions are made up of three 'ranges' that designate a range of numbers and an optional step.
Ranges can be written as:

* `5` - Only the number five (5)
* `-2..3` - The numbers negative two through three (-2, -1, 0, 1, 2, 3)
* `0..6%2` - The numbers zero through six with a step of two (0, 2, 4, 6)

These are then combined with a comma separating each range to make a region.  For example:

* `0,0,0` - A single node at the origin.
* `0,0,0..5` - A line of nodes running from the origin to five away on the z axis.
* `-1..1,-1..1,-1..1` - A cube of nodes that goes from negative one to one.
* `-12..12%3,5,-12..12%3` - A grid of nodes raised five from the origin with two empty nodes between each.

PING
----

Syntax: `PING`

Echoes back `PONG`.

STATUS
------

Syntax: `STATUS`

Prints information about the current state of the world and redpile's internal state.

MESSAGES
--------

Syntax: `MESSAGES`

Prints the list of messages currently 'in flight' between nodes.

NODE
----

Syntax: `NODE range`

Returns information about the node(s) in the specified range in the format `(x,y,z) type [fields]`.
Each field will be displayed in the format `name:value`.

Syntax: `NODE range type [fields]`

Sets the node(s) in the specified range to have a type of `type`.
In addition, any fields passed passed with the syntax `name:value` will be set on the node as well.
See [Types](types.md) for more information on types.

DELETE
------

Syntax: `DELETE range`

Removes any nodes inside the range provided

TICK
----

Syntax: `TICK [count]`

Runs a single redstone tick by default.
If `count` is provided, it will run `count` ticks.

The following are the possible outputs that can occur as the result of a tick:

* `(x,y,z) FIELD field value` - Set the field `field` on `(x,y,z)` to `value`
* `(x,y,z) PUSH direction` - Pushes the node at `(x,y,z)` in `direction`
* `(x,y,z) REMOVE` - Replaces the node with the default type

TICKV
-----

Syntax: `TICKV [count]`

Runs the `TICK` command with verbose output.

TICKQ
-----

Syntax: `TICKQ [count]`

Runs the `TICK` command but suppresses all output except errors.

