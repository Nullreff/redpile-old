Redpile [![Build Status](https://travis-ci.org/Nullreff/redpile.svg?branch=master)](https://travis-ci.org/Nullreff/redpile)
=======

**THIS IS A WORK IN PROGRESS**

Redpile is a voxel database and logic simulator.
You can use it to query and modify a grid of three dimensional data using simple text based commands.
All configuration and custom behavior is done using the Lua scripting language.
It can be run as a command line tool or as a server that talks via sockets.

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

* ~~Node storage~~
* ~~Command line interface~~
* ~~Sockets interface~~
* ~~Redstone Tests~~
* ~~Block message passing~~
* ~~Scripting via [Lua](http://www.lua.org/)~~
* ~~Field storage~~
* ~~Octree storage~~
* Storage optimization
* Message filtering
* Tick optimization
* Behaviors API
* Loading/saving
* [Tracing JIT](http://en.wikipedia.org/wiki/Tracing_just-in-time_compilation)
* Statistics & reporting
* Convert parts to [Rust](http://www.rust-lang.org/) (once rust is stable)

License
-------

* [Redpile](http://redpile.org/) - BSD License
* [Linenoise](https://github.com/antirez/linenoise/) - BSD License
* [Lua](http://www.lua.org/) - MIT License

