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
Run `./build.sh` to compile or `./test.sh` to run tests.

~~By default, Redpile will load redstone from a Minecraft map directory passed to it on the command line.~~ (WIP)
It then reads commands for block updates from the standard input and writes the resulting changes to the standard output.
For command line options, try `redpile --help`.

Commands
--------

**SET x y z material [direction]**

Sets the block at `(x, y, z)` to have a material of `material`.  Possible values for `material` are:

* EMPTY
* AIR
* WIRE
* CONDUCTOR
* INSULATOR
* TORCH

**GET x y z**

Returns information about the block at `(x, y, z)` in the format `(x,y,z) power material [direction]`.

**TICK**

Runs a single redstone tick.
Any blocks affected by the tick will be returned in the format `(x,y,z) power`.

Currently implemented are:

* WIRE - Propigates power
* TORCH - Powers wires and switches off when powered

**STATUS**

Prints information about the current state of the world.

* Ticks - The total number of times the `TICK` command has been called.
* Blocks - The total number of blocks with unique locations that have been added via `SET`.
* Power Sources - The total number of blocks that can potentailly supply power to others.
* Allocated Blocks - The number of blocks Redpile can store before it needs to allocate more memory.
* Allocated Buckets - The number of hash buckets Redpile uses to keep track of blocks.
* Bucket Collisions - How many collisions of hash values are there, lower is better.
* Max Bucket Depth - The maximum number of buckets Redpile will traverse to find a block, lower is better.

License
-------

Released under GPLv3, see LICENSE.txt for more information.

