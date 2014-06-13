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

Redpile reads commands for block updates from the standard input and writes the resulting changes to the standard output.
For command line options, try `redpile --help`.  Checkout `docs/commands` for more information on it's usage.

Roadmap
-------

If you're interested in helping, pull requests are always welcome.  The biggest thing we need right now are more tests of various redstone edge cases.

* ~~Block storage~~
* ~~Command line interface~~
* ~~Basic Redstone~~
* ~~Block message passing~~
* Field storage
* More tests!
* Tick caching
* Map loading/saving
* Restone [Tracing JIT](http://en.wikipedia.org/wiki/Tracing_just-in-time_compilation)
* Scripting
* Sockets interface
* Bukkit patch
* Language bindings
* Statistics & reporting
* Convert parts to [Rust](http://www.rust-lang.org/) (once rust is stable)

License
-------

Redpile is GPL, see LICENSE for more information.  
Linenoise is BSD, see [github.com/antirez/linenoise](https://github.com/antirez/linenoise)

