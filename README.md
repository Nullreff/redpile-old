Redpile [![Build Status](https://travis-ci.org/Nullreff/redpile.svg?branch=master)](https://travis-ci.org/Nullreff/redpile)
=======

**THIS IS A WORK IN PROGRESS**

Redpile is a high performance implementation of redstone for Minecraft.
It can be run from the command line or as a sub-process of a larger program.
When started, it creates a simulation of a Minecraft world focused solely on redstone.
Commands are sent on the standard input and block updates are received on the standard output.
It's intended to be very simple: You can set blocks, get blocks and run redstone ticks.

Building & Usage
----------------

Requires [CMake](http://www.cmake.org/) for building and [Rspec](http://rspec.info/) for CLI tests.
Run `make` to compile or `make test` to run tests.

~~By default, Redpile will load redstone from a Minecraft map directory passed to it on the command line.~~ (WIP)
It then reads commands for block updates from the standard input and writes the resulting changes to the standard output.
For command line options, try `redpile --help`.

Commands
--------

**SET x y z material [direction] [state]**

Sets the block at `(x, y, z)` to have a material of `material` and a direction of `direction`.

Values for `material` are:

* EMPTY
* AIR
* WIRE
* CONDUCTOR
* INSULATOR
* TORCH
* REPEATER
* COMPARATOR
* PISTON

Values for `direction` are:

* NORTH
* SOUTH
* EAST
* WEST
* UP
* DOWN

**GET x y z**

Returns information about the block at `(x, y, z)` in the format `(x,y,z) power material [direction] [state]`.  The value of `direction` is only returned if it's relevant to the current block.

**TICK**

Runs a single redstone tick.
Any blocks affected by the tick will be returned in the format `(x,y,z) power`.

Currently implemented are:

* AIR - Empty area that cannot be powered but allows power around it
* WIRE - Propagates power to adjacent blocks and those up/down one block
* CONDUCTOR - Block that can be powered
* INSULATOR - Block that cannot be powered
* TORCH - Powers wires and switches off when powered
* REPEATER - Powers a block in front if it receives power from the block behind.  Delays signal by `state + 1` ticks and locks up if powered by another repeater from the side.
* COMPARATOR - Combines the powers of the block behind it and the block to the side of it.  If `state > 1` it subtracts the side from behind.  Otherwise, it only propagates if the rear block's power is greater than the block to the side.
* PISTON - When powered, moves the block in front of it forwards one and inserts an INSULATOR block where the block used to be.  When unpowered, it move the block two in front of it back one and inserts an AIR block where the block used to be.

**STATUS**

Prints information about the current state of the world and redpile's internal state.

**PING**

Echoes back `PONG`.

License
-------

Redpile is GPL, see LICENSE for more information.  
Linenoise is BSD, see [github.com/antirez/linenoise](https://github.com/antirez/linenoise)

