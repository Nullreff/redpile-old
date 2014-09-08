Redpile [![Build Status](https://travis-ci.org/Nullreff/redpile.svg?branch=master)](https://travis-ci.org/Nullreff/redpile)
=======

**THIS IS A WORK IN PROGRESS**

Redpile is a rethink of Redstone for Minecraft with more flexibility, speed and utility.
It can be run from the command line or as a standalone server that talks via sockets.
For more information, check out [redpile.org](http://redpile.org/).

Building & Usage
----------------

Requires:

* C compiler
* Make
* CMake
* Bison
* Flex
* Rspec (tests)

Run `make` to compile or `make test` to run tests.

Redpile reads commands for block updates from the standard input and writes the resulting changes to the standard output.
For command line options, try `redpile --help`.  Checkout `docs/commands` for more information on it's usage.

Roadmap
-------

If you're interested in helping, pull requests are always welcome.  The biggest thing we need right now are more tests of various redstone edge cases.

* ~~Block storage~~
* ~~Command line interface~~
* ~~Sockets interface~~
* ~~Basic Redstone~~
* ~~Block message passing~~
* ~~Scripting via [Lua](http://www.lua.org/)~~
* ~~Field storage~~
* More tests!
* Tick caching
* Map loading/saving
* Restone [Tracing JIT](http://en.wikipedia.org/wiki/Tracing_just-in-time_compilation)
* Bukkit patch
* Language bindings
* Statistics & reporting
* Convert parts to [Rust](http://www.rust-lang.org/) (once rust is stable)

License
-------

[Redpile](http://redpile.org/) - BSD License
[Linenoise](https://github.com/antirez/linenoise/) - BSD License
[Lua](http://www.lua.org/) - MIT License
