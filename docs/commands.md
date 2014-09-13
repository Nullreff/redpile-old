Commands
========

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

GET
---

Syntax: `GET x y z`

Returns information about the node at `(x, y, z)` in the format `(x,y,z) type [fields]`.
Each field will be displayed in the format `name:value`.

SET
---

Syntax: `SET x y z type [fields]`

Sets the node at `(x, y, z)` to have a type of `type`.
In addition, any fields passed passed with the syntax `name:value` will be set on the node as well.
See [Types](types.md) for more information on types.

SETR
----

Syntax: `SETR x_start y_start z_start x_end y_end z_end type [fields]`

Runs a `SET` command on the range of nodes between `(x_start, y_start, z_start)` and `(x_end, y_end, z_end)`.

SETRS
----

Syntax: `SETRS x_start y_start z_start x_end y_end z_end x_step y_step z_step material [fields]`

Runs a `SET` command on the range of nodes between `(x_start, y_start, z_start)` and `(x_end, y_end, z_end)` incrementing by `(x_step, y_step, z_step)` every time.


DELETE
------

Syntax: `DELETE x y z`

Removes the node at `(x, y, z)`.

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

