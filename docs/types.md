Types
=====

**Note:** Some types are implemented differently from Minecraft and others have been combined into a single type.
The goal here is to provide generic and efficient types that can be modified to fit whatever system they are used in.

Possible directions are: `NORTH`, `SOUTH`, `EAST`, `WEST`, `UP` and `DOWN`

Materials
---------

### AIR

Empty area that cannot be powered but allows power around it.

### WIRE

Propagates power to adjacent nodes and those up/down one node.

### CONDUCTOR

Block that can be powered.

### INSULATOR

Block that cannot be powered.

### TORCH

Requires: `direction`

Powers adjacent wires and conductors above it.  It switches off when powered.

### PISTON

Requires: `direction`

Swaps the two nodes in front of it on one of the following conditions:

* It's not powered and the first node in front is air.
* It's powered and the second node in front of it is air.

### REPEATER

Requires: `direction` `state`

Powers a node in front if it receives power from the node behind.
Any signal passed through is delayed by `state` ticks.
If powered by another repeater from the side it will lock and not allow any signal through.

### COMPARATOR

Requires: `direction` `state`

Combines the powers of the node behind it and the node to the side of it.
If `state > 1` it subtracts the side from behind.
Otherwise, it only propagates if the rear node's power is greater than the node to the side.

