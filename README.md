Redpile [![Build Status](https://travis-ci.org/Nullreff/redpile.svg?branch=master)](https://travis-ci.org/Nullreff/redpile)
=======

Redpile is a high performance implementation of redstone for Minecraft.  It is currently a work in progress and should not be used for anything important.

Buiding
-------

Requireds [CMake](http://www.cmake.org/) for building and [Rspec](http://rspec.info/) for CLI tests.  Run `./build.sh` to compile or `./test.sh` to run tests.

Usage
-----

By default, Redpile will load redstone from a Minecraft map directory passed to it on the command line.  It then reads commands for block updates from STDIN and writes the resulting changes to STDOUT.  For command line options, try `redpile --help`.

Commands
--------

Commands are sent and received the format `COMMAND params`.  Any blocks affected by the command will be returned in the format `(x,y,z) material powerk

**SET x y z material**

Sets the block at `(x, y, z)` to have a material of `type`.

* 0 - EMPTY
* 1 - AIR
* 2 - WIRE
* 3 - CONDUCTOR
* 4 - INSULATOR
* 5 - TORCH

**GET x y z**

Gets information about the block at `(x, y, z)`.

**TICK**

Sets all torches to have a power of 15 and propagates the signal out to nearby wire, decreasing by one every time.

**STATUS**

Prints information about the current state of the world.

* Ticks - The total number of times the `TICK` command has been called.
* Blocks -  The total number of blocks with unique locations that have been added via `SET`.
* Allocated Blocks - The number of blocks Redpile can store before it needs to allocate more memory.
* Allocated Buckets - The number of hash buckets Redpile uses to keep track of blocks.
* Bucket Collisions - How many collisions of hash values are there.  Lower is better.
* Max Bucket Depth - The maximum number of buckets Redpile may have to traverse looking for a block.  Lower is better.

License
-------

Release under GPLv3, see LICENSE.txt for more information.
