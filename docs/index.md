Redpile
=======

Redpile is a high performance implementation of redstone for Minecraft.
It can be run from the command line or as a sub-process of a larger program.
When started, it creates a simulation of a Minecraft world focused solely on redstone.
Commands are sent on the standard input and block updates are received on the standard output.
It's intended to be very simple: You can set blocks, get blocks and run redstone ticks.

Installation
------------

*Note: Redpile is currently only supported on [Debian](https://www.debian.org/) as that's what my development machine runs.
Official support for other operating systems will come once it hits 1.0.0*

To install Redpile, open a shell and run the following commands:

~~~bash
sudo apt-get install git gcc build-essential cmake bison flex
git clone https://github.com/Nullreff/redpile.git
cd redpile
make
~~~

There should now be a `redpile` executable in the `build` directory.
The `master` branch should always build and pass all tests.
If it doesn't, please open an issue.

Usage
-----

For playing around, you can run redpile in `--interactive` (`-i`) mode.
See `--help` for a list of all options.

~~~bash
$ redpile -i conf/redstone.lua
>
~~~

You can set blocks, get blocks and run ticks.

~~~bash
> set 0 0 0 torch up
> get 0 0 0
(0,0,0) TORCH 0 UP
> set 0 0 1 wire
> set 0 0 2 wire
> tick
(0,0,0) POWER 0
(0,0,1) POWER 0
(0,0,2) POWER 0
> tick
(0,0,0) POWER 0
(0,0,2) POWER 14
(0,0,1) POWER 15
>
~~~

You can also print information about the current state of the world.

~~~bash
> status
ticks: 2
nodes: 21
hashmap_allocated: 1024
hashmap_overflow: 0
hashmap_resizes: 0
hashmap_max_depth: 0
message_max_inputs: 1
message_max_outputs: 4
message_max_queued: 1
~~~

For more commands, check out the [commands](commands.md) and [blocks](blocks.md) pages.

