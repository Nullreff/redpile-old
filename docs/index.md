Redpile
=======

Redpile is a voxel logic simulator.
You can use it to query and modify a grid of three dimensional data using simple text based commands.
All configuration and custom behavior is done using the Lua scripting language.
It can be run as a command line tool or as a server that talks via sockets.

Installation
------------

Redpile should compile and run just fine on any *nix system.
My development machine runs [Debian](https://www.debian.org/) so that will be the primary target.
Official support for other operating systems will come once it hits 1.0.0.

To install Redpile, open a shell and run the following commands.  Substitute `apt-get` for whatever package manager you use.

~~~bash
sudo apt-get install git gcc build-essential cmake bison flex
git clone https://github.com/Nullreff/redpile.git
cd redpile
make
sudo make install
~~~

You can also run tests to verify that everything is working:

~~~bash
sudo apt-get install ruby valgrind
sudo gem install rspec
make test    # Run all tests (fast)
make memtest # Run tests under valgrind (slow)
~~~

The `master` branch should always build and pass all tests.
If it doesn't, please open an issue.

Usage
-----

In order to run, Redpile needs a configuration file.
A file that simulates Redstone is provided in `conf/redstone.lua`.
For playing around, you can run redpile in `-i` (interactive) mode.
You can also have it listen on a specific port `-p <port>`.
See `--help` for a list of all options.

An example session in interactive mode is shown below.
For more information, check out the [commands](commands.md) and [types](types.md) pages.

~~~bash
$ redpile -i conf/redstone.lua 
> node 0,0,0 torch direction:up
> node 0,0,1..2 wire
> node 0,0,0..3
0,0,0 TORCH power:0 direction:UP
0,0,1 WIRE power:0
0,0,2 WIRE power:0
0,0,3 AIR
> tick 2
0,0,0 FIELD power:15
0,0,2 FIELD power:14
0,0,1 FIELD power:15
> message
0 0,0,0 => 0,0,1 POWER 15
> node 0,0,2
0,0,2 WIRE power:14
>
~~~

