Redpile - High Performance Redstone
=======

Redpile is a high performance implementation of redstone from Minecraft.  It is currently a work in progress and should not be used for anything important.  What follows is a specification for what I anticipate building.  None of this is actually implemented...  yet.

Usage
-----

By default, Redpile will load redstone from a Minecraft map directory passed to it on the command line.  It then reads commands for block updates from STDIN and writes the resulting commands to STDOUT.  

```text
Redpile - High Performance Redstone

Usage: redpile [options] [map directory]

Options:
    -i, --interactive
        Run in interactive mode with a prompt for reading commands

    -v, --version
        Print the current version

    -h, --help
        Print this message
```

Commands
--------

Commands are sent and received the format `Command(params, go, here)`.

**On(x, y, z)**

Sends power to the block at the coordinates `(x, y, z)`

**Off(x, y, z)**

Removes power from the block at the coordinates `(x, y, z)`

**Toggle(x, y, z)**

Toggles power in the block at the coordinates `(x, y, z)`

**Tick(count)**

Advances the redstone state by the number of ticks specified in `count`

