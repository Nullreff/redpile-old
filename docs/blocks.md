Blocks
======

**Note:** Some blocks are implemented differently from Minecraft and others have been combined into a single block.
The goal here is to provide generic and efficient blocks that can be modified to fit whatever system they are used in.

Directions
----------

* NORTH
* SOUTH
* EAST
* WEST
* UP
* DOWN

States
------

State is currently limited to the values 0-3.

Materials
---------

### AIR

Empty area that cannot be powered but allows power around it.

### WIRE

Propagates power to adjacent blocks and those up/down one block.

### CONDUCTOR

Block that can be powered.

### INSULATOR

Block that cannot be powered.

### TORCH

Requires: `direction`

Powers adjacent wires and conductors above it.  It switches off when powered.

### PISTON

Requires: `direction`

Swaps the two blocks in front of it on one of the following conditions:

* It's not powered and the first block in front is an air block
* It's powered and the second block in front of it is an air block

### REPEATER

Requires: `direction` `state`

Powers a block in front if it receives power from the block behind.
Any signal passed through is delayed by `state` ticks.
If powered by another repeater from the side it will lock and not allow any signal through.

### COMPARATOR

Requires: `direction` `state`

Combines the powers of the block behind it and the block to the side of it.
If `state > 1` it subtracts the side from behind.
Otherwise, it only propagates if the rear block's power is greater than the block to the side.

