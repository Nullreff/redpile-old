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

GET
---

Syntax: `GET x y z`

Returns information about the block at `(x, y, z)` in the format `(x,y,z) power material [direction] [state]`.
The values of `direction` and `state` are only returned if they are relevant to the current block.

SET
---

Syntax: `SET x y z material [direction] [state]`

Sets the block at `(x, y, z)` to have a material of `material`, a direction of `direction` and a state of `state`.
See [Blocks](../blocks.md) for more information.

TICK
----

Syntax: `TICK`

Runs a single redstone tick.  Will output the following updates:

* `POWER (x,y,z) power` - Set the power of `(x,y,z)` to `power`
* `STATE (x,y,z) state` - Set the state of `(x,y,z)` to `state`
* `SWAP (x1,y1,z1) (x2,y2,z2)` - Swap the blocks at `(x1,y1,z1)` and `(x2,y2,z2)`

