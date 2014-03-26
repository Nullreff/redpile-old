Redpile [![Build Status](https://travis-ci.org/Nullreff/redpile.svg?branch=master)](https://travis-ci.org/Nullreff/redpile)
=======

Redpile is a high performance implementation of redstone for Minecraft.  It is currently a work in progress and should not be used for anything important.  What follows is a specification for what I anticipate building.  None of this is actually implemented...  yet.

Buiding
-------

Requireds [CMake](http://www.cmake.org/) for building and [Rspec](http://rspec.info/) for CLI tests.  Run `./build.sh` to compile or `./test.sh` to run tests.

Usage
-----

By default, Redpile will load redstone from a Minecraft map directory passed to it on the command line.  It then reads commands for block updates from STDIN and writes the resulting commands to STDOUT.  For command line options, try `redpile --help`.

Commands
--------

Commands are sent and received the format `COMMAND params`.

**ADD x y z material**

Sets the block at `(x, y, z)` to have a material of `type`.

* 0 - EMPTY
* 1 - AIR
* 2 - WIRE
* 3 - CONDUCTOR
* 4 - INSULATOR

**SET x y z power**

Sets the block at `(x, y, z)` to have a power of `power`.

**TICK**

Advances the redstone state by one tick.

License
-------

Release under GPLv3, see LICENSE.txt for more information.
