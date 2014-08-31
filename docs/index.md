Redpile
=======

Redpile is a rethink of Redstone for Minecraft with more flexibility, speed and utility.
It can be run from the command line or as a standalone server that talks via sockets.

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
sudo make install
~~~

The `master` branch should always build and pass all tests.
If it doesn't, please open an issue.

Usage
-----

For playing around, you can run redpile in `-i` (interactive) mode.
You can also have it listen on a specific port `-p <port>`.
See `--help` for a list of all options.

~~~bash
$ redpile -i conf/redstone.lua
> set 0 0 0 torch direction:up
> get 0 0 0
(0,0,0) TORCH 0 direction:UP
> set 0 0 1 wire
> set 0 0 2 wire
> tick
(0,0,0) POWER 15
(0,0,1) POWER 0
(0,0,2) POWER 0
> tick
(0,0,0) POWER 15
(0,0,2) POWER 14
(0,0,1) POWER 15
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

For more commands, check out the [commands](commands.md) and [types](types.md) pages.

