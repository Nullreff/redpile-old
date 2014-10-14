Redpile [![Build Status](https://travis-ci.org/Nullreff/redpile.svg?branch=master)](https://travis-ci.org/Nullreff/redpile)
=======

**THIS IS A WORK IN PROGRESS**

Redpile is a voxel logic simulator.
You can think of it as a Redstone simulator with more flexibility, speed and utility.
It can be run from the command line or as a standalone server that talks via sockets.

Building & Usage
----------------

Requires:

* C compiler
* Make
* CMake
* Bison
* Flex
* Rspec (tests)
* Valgrind (tests)

Run `make` to compile or `make test` to run tests.  Run `make help` for a list of all commands.
For more information, check out [redpile.org](http://redpile.org/).

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
* Octree storage
* Tick caching
* Map loading/saving
* Restone [Tracing JIT](http://en.wikipedia.org/wiki/Tracing_just-in-time_compilation)
* Bukkit patch (or not)
* Language bindings
* Statistics & reporting
* Convert parts to [Rust](http://www.rust-lang.org/) (once rust is stable)

License
-------

* [Redpile](http://redpile.org/) - BSD License
* [Linenoise](https://github.com/antirez/linenoise/) - BSD License
* [Lua](http://www.lua.org/) - MIT License

